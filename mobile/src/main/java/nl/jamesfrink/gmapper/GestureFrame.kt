package nl.jamesfrink.gmapper


import android.annotation.SuppressLint
import android.os.Bundle
import android.support.v4.app.Fragment

import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.content.Context
import android.content.Intent
import android.util.AttributeSet
import android.util.Log


/**
 * A simple [Fragment] subclass.
 */

@SuppressLint("ValidFragment")
class GestureFrame( stateManager : StateManager) : Fragment(  ) {
    val debugTag = "GestureFrame"
    var gestureManager: GestureManager = GestureManager( stateManager )
    var gestureEditor = GestureEditor( stateManager )
    var currentFragment : Int = -1

    init {
        stateManager.gestureAdapter
    }

    fun setClassificationState( state : Int )
    {
        gestureManager.setClassificationState( state )
        gestureEditor.setClassificationState( state )
    }

    override fun onCreate( savedInstanceState: Bundle? ) {
        super.onCreate( savedInstanceState )
        retainInstance = true;

        //displayFragment( 1 ) // load in to memory
        displayFragment( 0 )

        Log.d(debugTag,"oncreate" )
    }

    override fun onCreateView(inflater: LayoutInflater?, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        // Inflate the layout for this fragment
        val view = inflater!!.inflate( R.layout.fragment_gesture_frame, container, false )
        Log.d(debugTag,"oncreateview" )
        return view
    }

    override fun onViewCreated(view: View?, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        Log.d(debugTag,"onViewCreated" )
    }

    fun displayFragment( fragmentNumber : Int )
    {
        if ( currentFragment != fragmentNumber ) {
            currentFragment = fragmentNumber
            Log.d(debugTag, "displayFragment" + fragmentNumber.toString())
            val fragment: Fragment? = when (fragmentNumber) {
                0 -> gestureManager
                1 -> gestureEditor
                else -> null
            }
            val cfm = activity.supportFragmentManager
            val t = cfm.beginTransaction()
            t.setCustomAnimations(R.anim.abc_popup_enter, R.anim.abc_popup_exit)
            t.replace(R.id.frameLayout_gestureFragments, fragment)
            t.commit()
        }
        //if ( fragmentNumber == 1 )
        //    t.addToBackStack( debugTag )
    }

    override fun onActivityCreated(savedInstanceState: Bundle?) {
        super.onActivityCreated(savedInstanceState)
        Log.d(debugTag,"onActivityCreated" )
    }

    override fun onAttach(context: Context?) {
        super.onAttach(context)
        Log.d(debugTag,"onAttach" )
    }

    override  fun onDetach() {
        super.onDetach()
        Log.d(debugTag,"onDetach" )
    }

    override fun onAttachFragment(childFragment: Fragment?) {
        super.onAttachFragment(childFragment)
        Log.d(debugTag,"onAttachfragment" )
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        Log.d(debugTag,"onActivityResult" )
    }

    override fun onDestroy() {
        super.onDestroy()
        Log.d(debugTag,"onDestroy" )
    }

    override fun onDestroyView() {
        super.onDestroyView()
        Log.d(debugTag,"onDestroyView" )
    }

    override fun onGetLayoutInflater(savedInstanceState: Bundle?): LayoutInflater {
        Log.d(debugTag,"onGetLayoutINflater" )
        return super.onGetLayoutInflater(savedInstanceState)
    }

    override fun onHiddenChanged(hidden: Boolean) {
        super.onHiddenChanged(hidden)
        Log.d(debugTag,"onHIddenChanged" )
    }

    override fun onStart() {
        super.onStart()
        Log.d(debugTag,"onStart" )
    }

    override fun onPause() {
        super.onPause()
        Log.d(debugTag,"onPause" )
    }

    override fun onStop() {
        super.onStop()
        Log.d(debugTag,"onStop" )
    }

    override fun onResume() {
        super.onResume()
        Log.d(debugTag,"onResume" )
    }

    override fun onSaveInstanceState(outState: Bundle?) {
        super.onSaveInstanceState(outState)
        Log.d(debugTag,"onSaveInstanceState" )
    }

    override fun onViewStateRestored(savedInstanceState: Bundle?) {
        super.onViewStateRestored(savedInstanceState)
        Log.d(debugTag,"onViewStateRestored" )
    }

    override fun onLowMemory() {
        super.onLowMemory()
        Log.d(debugTag, "onLowMemory" )
    }

    override fun onInflate(context: Context?, attrs: AttributeSet?, savedInstanceState: Bundle?) {
        super.onInflate(context, attrs, savedInstanceState)
        Log.d(debugTag,"onInflate" )
    }

}// Required empty public constructor
