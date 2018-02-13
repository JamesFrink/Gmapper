package nl.jamesfrink.gmapper


import android.annotation.SuppressLint
import android.os.Bundle
import android.support.v4.app.Fragment
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import kotlinx.android.synthetic.main.fragment_settings.*
import android.animation.ArgbEvaluator
import android.animation.ValueAnimator
import android.content.Context
import android.graphics.Color
import android.os.Handler
import android.support.constraint.ConstraintLayout
import android.view.inputmethod.InputMethodManager
import android.widget.*
import java.lang.Math.round

/**
 * A simple [Fragment] subclass.
 */
@SuppressLint("ValidFragment")
class Settings( val stateManager : StateManager) : Fragment(  ) {
    val settingsCallback = SettingsCallback( this )
    var useWear = false;

    val handler = Handler(  )
    val maxValSeekBar = 10000.0f
    val minGforce = 1.0f
    val maxGforce = 4.0f // To add on top of min
    val minTime = 5
    val maxTime = 2995 // 3 seconds total
    val maxCost = 500.0f

    var startForce = 2.0f
    var startForceTime = 100
    var stopForce = 1.2f
    var stopForceTime = 300
    var costThreshold = 100.0f

    var backGroundLinearLayout : LinearLayout? = null
    var useWearableSwitch : Switch? = null
    var sendIFTTTCheckBox : CheckBox? = null
    var webhookKeyIFTTTEditText : EditText? = null
    var sendOSCCheckBox : CheckBox? = null
    var oscHostEditText : EditText? = null
    var oscPortEditText : EditText? = null

    var viewSegmentOnThresholdLayout : ConstraintLayout? = null
    var viewSegmentOffThresholdLayout : ConstraintLayout? = null
    var thresholdStart : SeekBar? = null
    var thresholdStartText : TextView? = null
    var thresholdStartTime : SeekBar? = null
    var thresholdStartTimeText : TextView? = null
    var thresholdStop : SeekBar? = null
    var thresholdStopText : TextView? = null
    var thresholdStopTime : SeekBar? = null
    var thresholdStopTimeText : TextView? = null
    var thresholdCost : SeekBar? = null
    var thresholdCostText : TextView? = null

    var restoreDefaultsButton : Button? = null
    val colorAnimation : ValueAnimator = ValueAnimator.ofObject( ArgbEvaluator(  ), Color.argb( 255, 100, 255, 100 ), Color.argb( 142, 253, 253, 253 ) )

            // Callback from C++ (ZZLjava/lang/String;ZLjava/lang/String;IFIFI)V <<< signature
    fun setAllValues( useWearable : Boolean, sendIFTTT : Boolean, webhookKeyIFTTT : String,
                      sendOSC : Boolean, oscHost : String, oscPort : Int, startF : Float,
                      startFT : Int, stopF : Float, stopFT : Int, cThreshold : Float )
    {
        handler.post( object : Runnable {
            override fun run() {
                run {
                    // useWeareable
                    useWear = useWearable
                    if ( useWear )
                        stateManager.messageHandler.send( serialize( 1 ) )
                    else
                        stateManager.messageHandler.send( serialize( 0 ) )

                    useWearableSwitch?.isChecked = useWearable
                    Log.d( "useWearable", "Call watch function here as well" )
                    // sendIFTTT
                    sendIFTTTCheckBox?.isChecked = sendIFTTT
                    // webHookIFTTT
                    webhookKeyIFTTTEditText?.setText( webhookKeyIFTTT )
                    // sendOSC
                    sendOSCCheckBox?.isChecked = sendOSC
                    // oscHost
                    oscHostEditText?.setText( oscHost )
                    // oscPort
                    oscPortEditText?.setText( oscPort.toString(  ) )
                    // startForce
                    startForce = startF
                    thresholdStart?.progress = round( ( ( startForce - minGforce ) / maxGforce ) * maxValSeekBar )
                    var g = "%.2f G".format( startForce )
                    thresholdStartText?.text = g
                    // startForceTime
                    startForceTime = startFT
                    thresholdStartTime?.progress = round( ( ( startForceTime - minTime ) / maxTime.toFloat(  ) ) * maxValSeekBar )
                    var t = "%d ms".format( startForceTime )
                    thresholdStartTimeText?.text = t
                    // stopForce
                    stopForce = stopF
                    thresholdStop?.progress = round( ( ( stopForce - minGforce ) / maxGforce ) * maxValSeekBar )
                    g = "%.2f G".format( stopForce )
                    thresholdStopText?.text = g
                    // stopForceTime
                    stopForceTime = stopFT
                    thresholdStopTime?.progress = round( ( ( stopForceTime - minTime ) / maxTime.toFloat(  ) ) * maxValSeekBar )
                    t = "%d ms".format( stopForceTime )
                    thresholdStopTimeText?.text = t
                    // costThreshold
                    costThreshold = cThreshold
                    thresholdCost?.progress = round( ( costThreshold / maxCost ) * maxValSeekBar )
                    t = "%.2f Cost".format( costThreshold )
                    thresholdCostText?.text = t
                }
            }
        })
    }

    fun setClassificationState( state : Int )
    {
        if ( viewSegmentOnThresholdLayout != null )
        when ( state )
        {
            0 -> {
                viewSegmentOnThresholdLayout?.setBackgroundColor( Color.argb( 142, 253, 253, 253 ) )
                viewSegmentOffThresholdLayout?.setBackgroundColor( Color.argb( 142, 253, 253, 253 ) )
            }
            2 -> {
                colorAnimation.cancel(  )
                viewSegmentOnThresholdLayout?.setBackgroundColor( Color.argb( 75, 255, 100, 100 ) )
                viewSegmentOffThresholdLayout?.setBackgroundColor( Color.argb( 75, 255, 100, 100 ) )
            }
            -1 -> {
                colorAnimation.start(  )
            }
        }
    }

    override fun onCreate( savedInstanceState: Bundle? ) {
        super.onCreate( savedInstanceState )
        retainInstance = true
        colorAnimation.duration = 1250 // milliseconds
        colorAnimation.addUpdateListener( { animator ->
            viewSegmentOnThresholdLayout?.setBackgroundColor( animator.animatedValue as Int )
            viewSegmentOffThresholdLayout?.setBackgroundColor( animator.animatedValue as Int )
        } )
    }

    override fun onCreateView(inflater: LayoutInflater?, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        // Inflate the layout for this fragment
        val view = inflater!!.inflate( R.layout.fragment_settings, container, false )
        return view
    }

    override fun onViewCreated(view: View?, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        // Use wearable
        backGroundLinearLayout = backGround
        backGroundLinearLayout?.setOnClickListener( {
            val imm = context.getSystemService( Context.INPUT_METHOD_SERVICE ) as InputMethodManager
            imm.hideSoftInputFromWindow( view?.windowToken, 0 )
        } )

        useWearableSwitch = switch_useWatch
        useWearableSwitch?.setOnClickListener {
            useWearableSwitch?.requestFocus(  )
            useWear = useWearableSwitch!!.isChecked

            if ( useWear )
                stateManager.messageHandler.send( serialize( 1 ) )
            else
                stateManager.messageHandler.send( serialize( 0 ) )

            stateManager.settingsSetWearable( useWear )
            Log.d( "UseWearable", "clicked " + useWearableSwitch!!.isChecked.toString(  ) + " TODO: CALL WATCH FUNCTION HERE " )
        }
        // Send IFTTT
        sendIFTTTCheckBox = checkBox_IFTTT
        sendIFTTTCheckBox?.setOnClickListener {
            sendIFTTTCheckBox?.requestFocus(  )
            stateManager.settingsSetIFTTT( sendIFTTTCheckBox!!.isChecked, webhookKeyIFTTTEditText!!.text.toString(  ) )
            Log.d( "sendIFTTTCheckBox", "clicked " + sendIFTTTCheckBox!!.isChecked.toString(  ) + " TODO: CALL FUNCTION HERE " )
        }
        // webhookIFTTT
        webhookKeyIFTTTEditText = editText_webhookKey
        webhookKeyIFTTTEditText?.setOnEditorActionListener( { _, _, _ ->
            // Set IFTTT webhook
            Log.d( "IFTTTwebhook", "set " + webhookKeyIFTTTEditText?.text )
            stateManager.settingsSetIFTTT( sendIFTTTCheckBox!!.isChecked, webhookKeyIFTTTEditText!!.text.toString(  ) )
            val imm = context.getSystemService( Context.INPUT_METHOD_SERVICE ) as InputMethodManager
            imm.hideSoftInputFromWindow( view?.windowToken, 0 )
            webhookKeyIFTTTEditText?.clearFocus(  )

            false
        })
        webhookKeyIFTTTEditText?.onFocusChangeListener = View.OnFocusChangeListener { _, hasFocus ->
            if ( !hasFocus )
            {
                stateManager.settingsSetIFTTT(sendIFTTTCheckBox!!.isChecked, webhookKeyIFTTTEditText!!.text.toString())
                Log.d("IFTTTwebhook", "set " + webhookKeyIFTTTEditText?.text)
            }
        }
        // sendOSC
        sendOSCCheckBox = checkBox_OscSendEvent
        sendOSCCheckBox?.setOnClickListener {
            sendOSCCheckBox?.requestFocus(  )
            stateManager.settingsSetOSC( sendOSCCheckBox!!.isChecked, oscHostEditText!!.text.toString(  ), oscPortEditText!!.text.toString(  ).toInt(  ) )
            Log.d( "sendOSCCheckBox", "clicked " + sendOSCCheckBox!!.isChecked.toString(  ) + " TODO: CALL FUNCTION HERE " )
        }
        // oscHost
        oscHostEditText = editText_host
        oscHostEditText?.setOnEditorActionListener( { _, _, _ ->
            // Set OSC Host
            stateManager.settingsSetOSC( sendOSCCheckBox!!.isChecked, oscHostEditText!!.text.toString(  ), oscPortEditText!!.text.toString(  ).toInt(  ) )
            Log.d( "oscHostEditText", "set " + oscHostEditText?.text )
            val imm = context.getSystemService( Context.INPUT_METHOD_SERVICE ) as InputMethodManager
            imm.hideSoftInputFromWindow( view?.windowToken, 0 )
            oscHostEditText?.clearFocus(  )

            false
        })
        oscHostEditText?.onFocusChangeListener = View.OnFocusChangeListener { _, hasFocus ->
            if ( !hasFocus )
            {
                stateManager.settingsSetOSC( sendOSCCheckBox!!.isChecked, oscHostEditText!!.text.toString(  ), oscPortEditText!!.text.toString(  ).toInt(  ) )
                Log.d("oscHostEditText", "set " + oscHostEditText?.text)
            }
        }
        // oscPort
        oscPortEditText = editText_port
        oscPortEditText?.setOnEditorActionListener( { _, _, _ ->
            // Set OSC Host
            stateManager.settingsSetOSC( sendOSCCheckBox!!.isChecked, oscHostEditText!!.text.toString(  ), oscPortEditText!!.text.toString(  ).toInt(  ) )
            Log.d( "oscPortEditText", "set " + oscPortEditText?.text )
            val imm = context.getSystemService( Context.INPUT_METHOD_SERVICE ) as InputMethodManager
            imm.hideSoftInputFromWindow( view?.windowToken, 0 )
            oscPortEditText?.clearFocus(  )

            false
        })
        oscPortEditText?.onFocusChangeListener = View.OnFocusChangeListener { _, hasFocus ->
            if ( !hasFocus )
            {
                stateManager.settingsSetOSC( sendOSCCheckBox!!.isChecked, oscHostEditText!!.text.toString(  ), oscPortEditText!!.text.toString(  ).toInt(  ) )
                Log.d("oscPortEditText", "set " + oscPortEditText?.text)
            }
        }
        // Segmentation
        viewSegmentOnThresholdLayout = viewSegmentOnThreshold
        viewSegmentOffThresholdLayout = viewSegmentOffThreshold
        thresholdStart = seekBar_thresholdStart
        thresholdStartText = textView_thresholdStart
        thresholdStart?.setOnSeekBarChangeListener( object : SeekBar.OnSeekBarChangeListener {
            override fun onStopTrackingTouch(p0: SeekBar?) {
                stateManager.settingsSetSegmentation( startForce, startForceTime, stopForce, stopForceTime, costThreshold )
                Log.d("SEEK", "RELEASED" + startForce.toString() )
            }
            override fun onStartTrackingTouch(p0: SeekBar?) {}
            override fun onProgressChanged(p0: SeekBar?, p1: Int, p2: Boolean) {
                startForce = minGforce + ( p1 / maxValSeekBar ) * maxGforce
                if ( startForce <= stopForce ) {
                    startForce = stopForce + 0.01f
                    p0!!.progress = ( ( ( startForce - minGforce ) / maxGforce ) * maxValSeekBar ).toInt(  )
                }
                val g : String = "%.2f G".format( startForce )
                thresholdStartText?.text = g
            }
        } )
        thresholdStartTime = seekBar_thresholdTimeStart
        thresholdStartTimeText = textView_thresholdTimeStart
        thresholdStartTime?.setOnSeekBarChangeListener( object : SeekBar.OnSeekBarChangeListener {
            override fun onStopTrackingTouch(p0: SeekBar?) {
                stateManager.settingsSetSegmentation( startForce, startForceTime, stopForce, stopForceTime, costThreshold )
                Log.d("SEEK", "RELEASED" + startForceTime.toString() )
            }
            override fun onStartTrackingTouch(p0: SeekBar?) {}
            override fun onProgressChanged(p0: SeekBar?, p1: Int, p2: Boolean) {
                startForceTime = ( minTime + ( p1 / maxValSeekBar ) * maxTime ).toInt(  )
                val t : String = "%d ms".format( startForceTime )
                thresholdStartTimeText?.text = t
            }
        } )
        thresholdStop = seekBar_thresholdStop
        thresholdStopText = textView_thresholdStop
        thresholdStop?.setOnSeekBarChangeListener( object : SeekBar.OnSeekBarChangeListener {
            override fun onStopTrackingTouch(p0: SeekBar?) {
                stateManager.settingsSetSegmentation( startForce, startForceTime, stopForce, stopForceTime, costThreshold )
                Log.d("SEEK", "RELEASED" + stopForce.toString() )
            }
            override fun onStartTrackingTouch(p0: SeekBar?) {}
            override fun onProgressChanged(p0: SeekBar?, p1: Int, p2: Boolean) {
                stopForce = minGforce + ( p1 / maxValSeekBar ) * maxGforce
                if ( stopForce >= startForce ) {
                    stopForce = startForce - 0.01f
                    p0!!.progress = ( ( ( stopForce - minGforce ) / maxGforce ) * maxValSeekBar ).toInt(  )
                }
                val g : String = "%.2f G".format( stopForce )
                thresholdStopText?.text = g
            }
        } )
        thresholdStopTime = seekBar_thresholdTimeStop
        thresholdStopTimeText = textView_thresholdTimeStop
        thresholdStopTime?.setOnSeekBarChangeListener( object : SeekBar.OnSeekBarChangeListener {
            override fun onStopTrackingTouch(p0: SeekBar?) {
                stateManager.settingsSetSegmentation( startForce, startForceTime, stopForce, stopForceTime, costThreshold )
                Log.d("SEEK", "RELEASED" + stopForceTime.toString() )
            }
            override fun onStartTrackingTouch(p0: SeekBar?) {}
            override fun onProgressChanged(p0: SeekBar?, p1: Int, p2: Boolean) {
                stopForceTime = ( minTime + ( p1 / maxValSeekBar ) * maxTime ).toInt(  )
                val t : String = "%d ms".format( stopForceTime )
                thresholdStopTimeText?.text = t
            }
        } )
        thresholdCost = seekbar_thresholdCosts
        thresholdCostText = textView_thresholdCosts
        thresholdCost?.setOnSeekBarChangeListener( object : SeekBar.OnSeekBarChangeListener {
            override fun onStopTrackingTouch(p0: SeekBar?) {
                stateManager.settingsSetSegmentation( startForce, startForceTime, stopForce, stopForceTime, costThreshold )
                Log.d("SEEK", "RELEASED" + costThreshold.toString() )
            }
            override fun onStartTrackingTouch(p0: SeekBar?) {}
            override fun onProgressChanged(p0: SeekBar?, p1: Int, p2: Boolean) {
                costThreshold = ( p1 / maxValSeekBar ) * maxCost
                val t : String = "%.2f Cost".format( costThreshold )
                thresholdCostText?.text = t
            }
        } )

        restoreDefaultsButton = button_restoreDefaults
        restoreDefaultsButton?.setOnClickListener {
            val imm = context.getSystemService( Context.INPUT_METHOD_SERVICE ) as InputMethodManager
            imm.hideSoftInputFromWindow( view?.windowToken, 0 )
            startForce = 2.0f
            thresholdStart?.progress = round( ( ( startForce - minGforce ) / maxGforce ) * maxValSeekBar )
            var g = "%.2f G".format( startForce )
            thresholdStartText?.text = g
            // startForceTime
            startForceTime = 100
            thresholdStartTime?.progress = round( ( ( startForceTime - minTime ) / maxTime.toFloat(  ) ) * maxValSeekBar )
            var t = "%d ms".format( startForceTime )
            thresholdStartTimeText?.text = t
            // stopForce
            stopForce = 1.20f
            thresholdStop?.progress = round( ( ( stopForce - minGforce ) / maxGforce ) * maxValSeekBar )
            g = "%.2f G".format( stopForce )
            thresholdStopText?.text = g
            // stopForceTime
            stopForceTime = 300
            thresholdStopTime?.progress = round( ( ( stopForceTime - minTime ) / maxTime.toFloat(  ) ) * maxValSeekBar )
            t = "%d ms".format( stopForceTime )
            thresholdStopTimeText?.text = t
            // costThreshold
            costThreshold = 100.0f
            thresholdCost?.progress = round( ( costThreshold / maxCost ) * maxValSeekBar )
            t = "%.2f Cost".format( costThreshold )
            thresholdCostText?.text = t
            stateManager.settingsSetSegmentation( startForce, startForceTime, stopForce, stopForceTime, costThreshold )
        }

        // Call setAllValues from C++
        stateManager.loadSettings( settingsCallback )
    }

}// Required empty public constructor

