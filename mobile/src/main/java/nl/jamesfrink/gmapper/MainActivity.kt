package nl.jamesfrink.gmapper

import android.support.v7.app.AppCompatActivity
import android.os.Bundle
import android.content.Intent
import android.os.PersistableBundle
import android.util.Log
import kotlinx.android.synthetic.main.activity_main.*
//import DataLayerListenerService


class MainActivity : AppCompatActivity(  ) {

    lateinit var stateManager : StateManager

    override fun onCreate( savedInstanceState: Bundle? ) {
        Log.d("main","activity_onCreate")
        super.onCreate( savedInstanceState )
        setContentView( R.layout.activity_main )
        if ( savedInstanceState == null ) {
            Log.d("main","savestate = null")
            stateManager = StateManager(this, supportFragmentManager)
        }
        viewPager_Fragments.offscreenPageLimit = 3 // would like to manage what gets deleted
        //stateManager.pagerAdapter.onCreate(  )
        viewPager_Fragments.adapter = stateManager.pagerAdapter

        gestureManager.setOnClickListener( {
            stateManager.pagerAdapter.gestureFrame.displayFragment( 0 )
            viewPager_Fragments.currentItem = 0
            Log.d("a", "b")

        } )
        eventManager.setOnClickListener({
            stateManager.pagerAdapter.gestureFrame.displayFragment( 0 )
            viewPager_Fragments.currentItem = 1
            Log.d("a", "c")
        } )
        settings.setOnClickListener({
            stateManager.pagerAdapter.gestureFrame.displayFragment( 0 )
            viewPager_Fragments.currentItem = 2
            Log.d("a", "d")
        } )
    }

    override fun onBackPressed(  )
    {
        // Don't quit the app on back pressed
        when {
            supportFragmentManager.backStackEntryCount > 0 -> super.onBackPressed()
            stateManager.pagerAdapter.gestureFrame.currentFragment == 1 -> stateManager.pagerAdapter.gestureFrame.displayFragment( 0 )
            else -> {   // Give the impression of exiting the app
                val startMain = Intent( Intent.ACTION_MAIN )
                startMain.addCategory( Intent.CATEGORY_HOME )
                startMain.flags = Intent.FLAG_ACTIVITY_NEW_TASK
                startActivity( startMain )
            }
        }
    }

    override fun onSaveInstanceState( outState: Bundle?, outPersistentState: PersistableBundle? ) {
        super.onSaveInstanceState( outState, outPersistentState )
        Log.d("main","onSaveInstance" )
    }

    override fun onResume(  ) {
        super.onResume(  )

    }

    override fun onPause(  ) {
        super.onPause(  )
        Log.d("main","onpause" )
    }

    override fun onStart() {
        super.onStart()
        Log.d("main","onstart" )
    }

    override fun onStop() {
        super.onStop()
        Log.d("main","onstop" )
    }

    override fun onRestart() {
        super.onRestart()
        Log.d("main","restart" )
    }

    override fun onStateNotSaved() {
        super.onStateNotSaved()
        Log.d("main","statenotsaved" )
    }

    override fun onDestroy() {
        super.onDestroy(  )
        stateManager.onDestroy(  )
        Log.w("main","destroy" )
    }

    override fun onRestoreInstanceState(savedInstanceState: Bundle?) {
        super.onRestoreInstanceState(savedInstanceState)
        Log.w("main","onRestoreInstanceState" )
    }

    override fun onPostResume() {
        super.onPostResume()
        Log.w("main","onPostResume" )
    }
}