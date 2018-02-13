package nl.jamesfrink.gmapper

import android.app.Activity
import android.content.Context
import android.util.Log
import com.google.android.gms.common.ConnectionResult
import com.google.android.gms.tasks.Tasks
import com.google.android.gms.wearable.*
import java.lang.Thread.sleep
import com.google.android.gms.common.GoogleApiAvailability





/**
 * Created by james on 28/01/2018.
 */


class MessageHandler ( val context : Context )
{
    var listenCallback : ( ( it : com.google.android.gms.wearable.MessageEvent ) -> Unit )? = null
    var connected = false
    val GMAP_MESSAGE_PATH = "/GMAP"
    val GMAP_CAPABILITY_NAME = "GMAP"
    private var gmapNodeId : String? = null
    private var capabilityClient : CapabilityClient? = null
    private var messageClient : MessageClient? = null
    init {

        Thread( Runnable { setupGMAP(  ) } ).start(  )
    }

    private fun setupGMAP (  ) {
        messageClient = Wearable.getMessageClient( context )
        capabilityClient = Wearable.getCapabilityClient( context )
        capabilityClient?.addLocalCapability( GMAP_CAPABILITY_NAME ) // TODO: remove on shutdown


        while ( gmapNodeId == null )
        {
            Log.w("ISNULL","ISNULLL")
            sleep( 600 )
            val capabilityInfo = Tasks.await(
                    capabilityClient!!.getCapability(
                            GMAP_CAPABILITY_NAME, CapabilityClient.FILTER_REACHABLE ) )
            // capabilityInfo has the reachable nodes with the GMAPPER capability
            updateGMAPCapability( capabilityInfo )
        }
        connected = true

        messageClient?.addListener {
            listenCallback?.invoke( it )
        }

    }

    private fun updateGMAPCapability(capabilityInfo: CapabilityInfo) {
        val connectedNodes = capabilityInfo.nodes
        gmapNodeId = pickBestNodeId( connectedNodes )
    }

    private fun pickBestNodeId(nodes: Set< Node >): String? {
        var bestNodeId: String? = null
        // Find a nearby node or pick one arbitrarily
        for ( node in nodes ) {
            if ( node.isNearby) {
                return node.id
            }
            bestNodeId = node.id
        }
        return bestNodeId
    }

    fun send( data : ByteArray ) {
        var RetryCountDown = 1000 // TODO: should be handled by listener to capability and not by retrying indefinitely
        Thread( Runnable {
            if ( gmapNodeId != null ) {
                val result = messageClient?.sendMessage( gmapNodeId!!, GMAP_MESSAGE_PATH, data )
                result?.addOnFailureListener( {
                    Log.d("FAILED", "FAILED")
                    connected = false
                } )

            } else {
                while ( gmapNodeId == null ) {
                    sleep( 1000 )
                    if ( --RetryCountDown < 0 )
                        break
                }
                if ( gmapNodeId != null )
                    messageClient?.sendMessage( gmapNodeId!!, GMAP_MESSAGE_PATH, data )
            }
        } ).start(  )
    }

    fun onDestroy ( )
    {
        capabilityClient?.removeLocalCapability( GMAP_CAPABILITY_NAME )
    }
}