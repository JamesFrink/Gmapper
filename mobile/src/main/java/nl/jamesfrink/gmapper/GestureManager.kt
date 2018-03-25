package nl.jamesfrink.gmapper

import android.os.Bundle
import android.support.design.widget.FloatingActionButton
import android.support.v4.app.Fragment
import android.support.v7.widget.LinearLayoutManager
import android.support.v7.widget.RecyclerView
import android.support.v7.widget.helper.ItemTouchHelper
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.support.v7.widget.DividerItemDecoration
import android.support.v7.app.AlertDialog
import android.text.InputType
import android.widget.EditText
import android.annotation.SuppressLint
import android.content.Context
import android.content.Intent
import android.graphics.Color
import android.os.Handler
import android.util.AttributeSet
import android.util.Log
import android.view.inputmethod.InputMethodManager


@SuppressLint("ValidFragment")
class GestureManager ( val stateManager : StateManager) : Fragment(  )
{
    val debugTag = "GestureMapper"

    var int = 0
    var initialized = false

    private lateinit var recyclerView : RecyclerView
    private lateinit var itemTouchHelper : ItemTouchHelper

    override fun onCreate( savedInstanceState: Bundle? ) {
        super.onCreate( savedInstanceState )
        //retainInstance = true;
        //this.stateManager = ( activity as MainActivity ).stateManager
        //parentFragment = stateManager.pagerAdapter!!.gestureFrame

        val simpleCallback = object : SwipeToDelete( context )
        {
            override fun onSwiped(viewHolder: RecyclerView.ViewHolder?, direction: Int)
            {
                val position = viewHolder!!.adapterPosition //get position which is swipe
                stateManager.removeGesture( position )
            }
        }
        itemTouchHelper = ItemTouchHelper( simpleCallback )

    }

    fun setClassificationState( state : Int ) {
        if ( initialized )
        when ( state )
        {
            0 -> recyclerView.setBackgroundColor( Color.TRANSPARENT )
            2 -> recyclerView.setBackgroundColor( Color.argb( 75, 255, 100, 100 ) )
        }
    }

    override fun onStart(  ) {
        super.onStart(  )
        Log.d(debugTag,"onStart" )
    }

    override fun onCreateView( inflater: LayoutInflater?, container: ViewGroup?,
                              savedInstanceState: Bundle? ): View? {

        // Inflate the layout for this fragment
        return inflater!!.inflate(R.layout.fragment_gesture_manager, container, false)
    }

    override fun onViewCreated(view: View?, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        // Get recyclerview
        recyclerView = view!!.findViewById< RecyclerView >( R.id.recyclerView_gestures )
        itemTouchHelper.attachToRecyclerView( recyclerView )

        val layoutManager = LinearLayoutManager( view.context )
        recyclerView.layoutManager = layoutManager
        recyclerView.swapAdapter( stateManager.sharedGestureList.rAdapter, true )
        recyclerView.addItemDecoration( DividerItemDecoration( recyclerView.context, layoutManager.orientation ) )


        // Get floating button
        val floatButton = view.findViewById< FloatingActionButton >( R.id.FloatingButton_addNewGesture )
        floatButton.setOnClickListener( {
            val builder = AlertDialog.Builder( ( activity as MainActivity ) )
            builder.setTitle( "Add new Gesture" )
            // Set up the input
            val input = EditText( context  )
            input.hint = "Gesture name here"
            // Specify the type of input expected; this, for example, sets the input as a password, and will mask the text
            input.inputType = InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_VARIATION_NORMAL

            input.requestFocus(  )
            builder.setView( input )

            // Set up the buttons
            builder.setPositiveButton( "OK" )
            {
                _, _ -> stateManager.addNewGesture( input.text.toString(  ) )
                int++
            }
            builder.setNegativeButton( "Cancel" ) {
                dialog, which -> dialog.cancel(  )
            }

            val imm = context.getSystemService( Context.INPUT_METHOD_SERVICE ) as InputMethodManager
            builder.show(  )
            Handler(  ).postDelayed({
                imm.showSoftInput( input, 0 );
            }, 100) // Delay to get around the silly keyboard stuff in Android ( WHY IS THIS PLATFORM SO SHABBY )
        } )
        initialized = true
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

}
