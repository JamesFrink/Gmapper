package nl.jamesfrink.gmapper

/**
 * Created by james on 30/01/2018.
 */
class SettingsCallback( val settings : Settings ) {
    fun setAllValues( useWearable : Boolean, sendIFTTT : Boolean, webhookKeyIFTTT : String,
                        sendOSC : Boolean, oscHost : String, oscPort : Int, startF : Float,
                        startFT : Int, stopF : Float, stopFT : Int, costThreshold : Float )
    { // Stupid workaround because of fragment inheritance issue and JNI
        settings.setAllValues( useWearable, sendIFTTT, webhookKeyIFTTT,
                sendOSC, oscHost, oscPort, startF, startFT, stopF, stopFT, costThreshold )
    }
}