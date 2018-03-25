package nl.jamesfrink.gmapper

/**
 * Created by james on 18/01/2018.
 */

import android.content.Context
import android.content.Context.SENSOR_SERVICE
import android.hardware.Sensor
import android.hardware.SensorEvent
import android.hardware.SensorEventListener
import android.hardware.SensorManager
import android.os.Handler
import android.support.v4.app.FragmentManager
import android.util.Log
import java.io.ByteArrayInputStream
import java.io.ObjectInputStream

fun deserialize( data: ByteArray ): Any {
    val `in` = ByteArrayInputStream( data )
    val `is` = ObjectInputStream( `in` )
    return `is`.readObject(  )
}

/* Initializes C++ model class and manages all other data */
// Interface to native codebase

class StateManager ( val context: Context, val fragmentManager: FragmentManager ) : SensorEventListener {
    val sharedGestureList : ArrayListAdapter<Gesture> = ArrayListAdapter()
    var gestureAdapter : GestureAdapter = GestureAdapter( this, sharedGestureList )
    lateinit var pagerAdapter : PagerAdapter
    var handler = Handler(  )


    external fun loadSettings ( settingsToSet : SettingsCallback ) : Boolean
    external fun settingsSetWearable ( useWear : Boolean ) : Boolean
    external fun settingsSetIFTTT ( sendIFTTT : Boolean, webhookKey : String ) : Boolean
    external fun settingsSetOSC ( sendOSC : Boolean, hostOSC : String, portOSC : Int ) : Boolean
    external fun settingsSetSegmentation ( startF : Float, startFT : Int, stopF : Float, stopFT : Int, costThreshold : Float ) : Boolean

    // Update to reflect the correct object
    external fun addNewGesture ( nameOfGesture : String ) : Boolean
    external fun setNameOfGesture ( index : Int, nameOfGesture : String ) : Boolean
    external fun removeGesture ( index : Int ) : Boolean
    external fun setGestureActive ( index : Int, active : Boolean ) : Boolean
    external fun clearAllExampleData ( index : Int ) : Boolean
    external fun removeTrainingSeriesObject ( indexOfGesture : Int, indexOfTrainingSeriesObject : Int ) : Boolean
    external fun waitForExampleData ( index : Int ) : Boolean
    external fun setClassificationState ( input : Boolean ) : Boolean
    //external fun getNumGestures (  ) : Int
    //external fun getGesture ( id : Int ) : nl.jamesfrink.gmapper.Gesture

    //external fun getGestureByIndex ( index : Int ) : nl.jamesfrink.gmapper.Gesture
    //external fun getTrainingSeriesObjectByIndex ( index : Int ) : nl.jamesfrink.gmapper.TrainingSeriesObject

    private external fun initializeJNI(listToShare : ArrayListAdapter<*>, directory : String ) : Boolean
    private external fun addAccelData( x : Float, y : Float, z : Float ) : Boolean
    private external fun destroyJNI(  ) : Boolean

    private var sensorManager : SensorManager
    private var gestureEditor : GestureEditor
    //private var dataLayerListenerService : DataLayerListenerService
    //var messageHandler = MessageHandler( context )

    init {
        sharedGestureList.setAdapter( gestureAdapter )
        sensorManager = context.getSystemService( SENSOR_SERVICE ) as SensorManager
        sensorManager.registerListener(
                this,
                sensorManager.getDefaultSensor( Sensor.TYPE_ACCELEROMETER ),
                SensorManager.SENSOR_DELAY_NORMAL )

        val filesDir = context.applicationContext.filesDir.toString(  );
        Log.d("THIS", filesDir)

        if ( !initializeJNI( sharedGestureList, filesDir ) )
            Log.w( "JNI_INITIALIZER", "COULD NOT INITIALIZE JNI OF STATEMANAGER" )

        pagerAdapter = PagerAdapter( fragmentManager, this )
        gestureEditor = pagerAdapter.gestureFrame.gestureEditor

        /*
        messageHandler.listenCallback = {
            when ( it.path )
            {
                "/GMAP" -> {
                    if ( pagerAdapter.settings.useWear ) {
                        var (x, y, z) = deserialize(it.data) as FloatArray
                        addAccelData(x, y, z)
                        pagerAdapter.sensorViewer.addAccelData( x, y, z )
                    }
                }
            }
        }
        */
    }

    companion object
    {
        // Used to load the C++ library on application startup.
        init
        {
            Log.d( "init", "LOADING JNI" )
            System.loadLibrary( "StateManagerJNI" )
        }
    }

    override fun onAccuracyChanged (sensor: Sensor?, accuracy: Int ) { }

    override fun onSensorChanged ( event: SensorEvent? )
    {   // Queue accel data to be processed in model thread
        if ( !pagerAdapter.settings.useWear )
        {
            addAccelData( event!!.values[0], event.values[1], event.values[2] )
            pagerAdapter.sensorViewer.addAccelData( event!!.values[0], event.values[1], event.values[2] )
        }
    }

    fun onStop ( )
    {
        //messageHandler.send( serialize( 0 ) )
    }

    fun onDestroy (  )
    {
        sharedGestureList.clear(  )
        if ( !destroyJNI(  ) )
            Log.d( "nl.jamesfrink.gmapper.StateManager", "Did not destroy" )
        //messageHandler.send( serialize( 0 ) )
        //messageHandler.onDestroy(  )
    }

    private fun classifiedGestureCallback ( gestureIndex : Int )
    { // C++ callback, could call from C++, but might want more data on java side
        handler.post( object : Runnable {
            override fun run() {
                run {
                    sharedGestureList[ gestureIndex ].triggerRecognized(  )
                    pagerAdapter.settings.setClassificationState( -1 ) // Highlight detection ( for checing costs )
                }
            }
        } )
    }

    private fun setCurrentClassificationState ( state : Int )
    { // C++ Callback
        handler.post( object : Runnable {
            override fun run() {
                run {
                    pagerAdapter.gestureFrame.setClassificationState( state )
                    pagerAdapter.settings.setClassificationState( state )
                }
            }
        } )
    }

    private fun getGestureEditorFocus ( gestureIndex : Int )
    { // C++ Callback
        handler.post( object : Runnable {
            override fun run(  ) {
                run {
                    pagerAdapter.gestureFrame.gestureEditor.setNewGesture( sharedGestureList[ gestureIndex ], gestureIndex )
                    pagerAdapter.gestureFrame.displayFragment( 1 )
                }
            }
        } )
    }
}


