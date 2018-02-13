package nl.jamesfrink.gmapper

import android.os.Bundle
import android.support.wearable.activity.WearableActivity
import android.widget.TextView
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : WearableActivity(  ) {
    private lateinit var sensorHandler : SensorHandler
    private lateinit var accelViewText : TextView
    //private lateinit var messageClient : MessageClient

    override fun onCreate( savedInstanceState: Bundle? ) {
        super.onCreate( savedInstanceState )
        setContentView( R.layout.activity_main )
        sensorHandler = SensorHandler( this )

        accelViewText = textView_accelView
        sensorHandler.sensorChangedListener = { x, y, z ->
            val debugView = "X: $x Y: $y Z: $z \n " + sensorHandler.sendAccelData.toString(  )
            accelViewText.text = debugView
        }

        //messageClient = MessageClient( this, this.application )

        // Enables Always-on
        setAmbientEnabled(  )
    }
}
