package nl.jamesfrink.gmapper

import android.support.v4.app.Fragment
import android.support.v4.app.FragmentManager
import android.support.v4.app.FragmentPagerAdapter

/**
 * Created by james on 20/01/2018.
 */

class PagerAdapter( fm: FragmentManager, stateManager: StateManager) : FragmentPagerAdapter( fm ) {

    val gestureFrame = GestureFrame( stateManager )
    val sensorViewer = SensorViewer(  )
    val settings = Settings( stateManager )
    private val numFragments = 3

    override fun getCount(  ): Int = numFragments

    override fun getItem( position: Int ) : Fragment?
    {
        return when( position )
        {
            0 -> gestureFrame // Don't reload this all the time
            1 -> sensorViewer
            2 -> settings
            else -> return null
        }
    }
}