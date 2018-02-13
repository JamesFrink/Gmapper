
import android.content.Context
import android.util.Log
import com.google.android.gms.wearable.*
import java.io.ByteArrayInputStream
import java.io.ObjectInputStream
import java.util.logging.Handler

fun deserialize( data: ByteArray ): Any {
    val `in` = ByteArrayInputStream( data )
    val `is` = ObjectInputStream( `in` )
    return `is`.readObject(  )
}

class DataLayerListenerService ( val context : Context)
{
    private val GMAP_MESSAGE_PATH = "/GMAP_STATE"
    private var messageClient : MessageClient = Wearable.getMessageClient( context )

    var callbackState : ( ( state : Int ) -> Unit )? = null
    init {
        messageClient.addListener { listenFunc( it ) }
    }

    private fun listenFunc ( message : MessageEvent )
    {
        when ( message.path )
        {
            GMAP_MESSAGE_PATH -> {
                val result : Int = deserialize( message.data ) as Int
                callbackState?.invoke( result )
            }
        }
    }

    fun destroy(  )
    {
        messageClient.removeListener { listenFunc( it ) }
    }
}