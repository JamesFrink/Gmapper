package nl.jamesfrink.gmapper

import kotlin.properties.Delegates

/**
 * Created by james on 24/01/2018.
 */
data class TrainingSeriesObject( private val lengthOfGesture : Int, var timeStamp : String, val numTimesRecognized_init : Int )
{
    var numTimesRecognizedListener : ( ( Int ) -> Unit )? = null

    var numTimesRecognized : Int by Delegates.observable( numTimesRecognized_init )
    { _, _, new ->
        numTimesRecognizedListener?.invoke( new )
    }

    var gestureTimeLength : String
    init {
        val milliseconds : String = ( lengthOfGesture % 1000 ).toString(  ).padStart( 3, '0' )
        val intermediateSeconds : Int = ( lengthOfGesture / 1000 )
        val seconds : String = ( intermediateSeconds % 60 ).toString(  ).padStart( 2, '0' )
        val minutes : String = ( intermediateSeconds / 60 ).toString(  ).padStart( 2, '0' )
        gestureTimeLength = "$minutes:$seconds:$milliseconds"
    }
}