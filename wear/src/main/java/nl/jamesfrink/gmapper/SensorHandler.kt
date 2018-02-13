package nl.jamesfrink.gmapper

import android.content.Context
import android.hardware.Sensor
import android.hardware.SensorEvent
import android.hardware.SensorEventListener
import android.hardware.SensorManager
import java.io.ByteArrayOutputStream
import java.io.ObjectOutputStream
import DataLayerListenerService
import com.google.android.gms.wearable.DataApi
import com.google.android.gms.wearable.DataItem
import deserialize
import java.util.logging.Handler

/**
 * Created by james on 28/01/2018.
 */


fun serialize( obj: Any ): ByteArray {
    val out = ByteArrayOutputStream(  )
    val os = ObjectOutputStream( out )
    os.writeObject( obj )
    return out.toByteArray()
}

class SensorHandler ( context : Context ) : SensorEventListener
{
    var sendAccelData = false;
    private var sensorManager : SensorManager = context.getSystemService(android.content.Context.SENSOR_SERVICE) as SensorManager
    var sensorChangedListener : ( ( x : Float, y : Float, z : Float ) -> Unit )? = null
    val messageHandler = MessageHandler( context )
    lateinit var useWatch : DataItem

    init {
        messageHandler.listenCallback = { state ->
            val state_num = deserialize( state.data ) as Int
            when ( state_num )
            {
                0-> sendAccelData = false
                1-> sendAccelData = true
            }
            sensorChangedListener?.invoke( 0.0f, 0.0f, 0.0f )
        }
        sensorManager.registerListener(
                this,
                sensorManager.getDefaultSensor( android.hardware.Sensor.TYPE_ACCELEROMETER ),
                SensorManager.SENSOR_DELAY_NORMAL )
    }

    override fun onAccuracyChanged(p0: Sensor?, p1: Int) {
    }

    override fun onSensorChanged(event: SensorEvent?) {
        if ( messageHandler.connected && sendAccelData )
        {
            sensorChangedListener?.invoke(event!!.values[0], event.values[1], event.values[2])
            messageHandler.send(serialize(event!!.values))
        }
    }
}
