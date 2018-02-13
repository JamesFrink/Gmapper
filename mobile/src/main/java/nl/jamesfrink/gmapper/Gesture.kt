package nl.jamesfrink.gmapper

import kotlin.properties.Delegates

/**
 * Created by james on 19/01/2018.
 */

data class Gesture ( var init_id : Int, var init_active : Boolean, var init_nameOfGesture : String, var init_numberOfRecordings : Int )
{
    var trainingSeriesObjects = ArrayListAdapter<TrainingSeriesObject>()
    var recognizedCallback : ( (  ) -> Unit )? = null
    var idListener : ( ( Int ) -> Unit )? = null
    var activeListener : ( ( Boolean ) -> Unit )? = null
    var nameOfGestureListener : ( ( String ) -> Unit )? = null
    var numberOfRecordingsListener : ( ( Int ) -> Unit )? = null

    var id : Int by Delegates.observable( 0 )
    { _, _, new ->
        idListener?.invoke( new )
    }

    var active : Boolean by Delegates.observable( false )
    { _, _, new ->
        activeListener?.invoke( new )
    }

    var nameOfGesture : String by Delegates.observable( "None" )
    { _, _, new ->
        nameOfGestureListener?.invoke( new )
    }

    var numberOfRecordings : Int by Delegates.observable( 0 ) {
        _, _, new ->
        numberOfRecordingsListener?.invoke( new )
    }

    init {
        id = init_id
        active = init_active
        nameOfGesture = init_nameOfGesture
        numberOfRecordings = init_numberOfRecordings
        trainingSeriesObjects.setAdapter( TrainingSeriesObjectAdapter( trainingSeriesObjects ) )
    }

    fun triggerRecognized(  )
    {
        recognizedCallback?.invoke(  )
    }

    fun unregisterAllListeners(  )
    {
        idListener = null
        activeListener = null
        nameOfGestureListener = null
        numberOfRecordingsListener = null
    }

    fun update ( new_id : Int, new_active : Boolean, new_nameOfGesture : String, new_numberOfRecordings : Int )
    {
        id = new_id
        active = new_active
        nameOfGesture = new_nameOfGesture
        numberOfRecordings = new_numberOfRecordings
    }
}