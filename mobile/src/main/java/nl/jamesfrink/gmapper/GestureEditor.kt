package nl.jamesfrink.gmapper


import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.support.design.widget.TextInputEditText
import android.support.v4.app.Fragment
import kotlinx.android.synthetic.main.fragment_gesture_editor.view.*
import android.annotation.SuppressLint
import android.content.Context
import android.content.Intent
import android.graphics.Color
import android.os.Handler
import android.support.constraint.ConstraintLayout
import android.support.v7.widget.DividerItemDecoration
import android.support.v7.widget.LinearLayoutManager
import android.support.v7.widget.RecyclerView
import android.support.v7.widget.helper.ItemTouchHelper
import android.util.AttributeSet
import android.util.Log
import android.view.KeyEvent
import android.view.inputmethod.InputMethodManager
import android.view.View.OnFocusChangeListener
import android.widget.*


/**
 * A simple [Fragment] subclass.
 */
@SuppressLint( "ValidFragment" )
class GestureEditor( val stateManager : StateManager) : Fragment(  ) {
    val debugTag = "GestureEditor"

    private var indexOfGesture = -1;

    private var handler = Handler(  )
    private lateinit var runnable : Runnable

    private lateinit var scrollView : ScrollView
    private lateinit var constraintLayout : ConstraintLayout
    private lateinit var textEdit : TextInputEditText
    private lateinit var switch : Switch
    private lateinit var numExamples : TextView
    private lateinit var buttonGetTrainingData: Button
    private lateinit var buttonClearAllTrainingData : Button
    private lateinit var trainingExamplesView : RecyclerView
    private lateinit var itemTouchHelper : ItemTouchHelper

    var gesture : Gesture? = null
    var initialized = false

    init {
        view?.setOnKeyListener( { _, keyCode, _ ->
            if ( keyCode == KeyEvent.KEYCODE_BACK ) {
            stateManager.waitForExampleData( -1 ) // Stop recording of example if that was happening
            true
        } else
            false
        } )
    }

    fun setClassificationState( state : Int )
    {
        if ( initialized )
        when ( state )
        { // 1, 3, 5
            0 -> {
                trainingExamplesView.setBackgroundColor(Color.TRANSPARENT)
                buttonGetTrainingData.text = "Collect Data"
                buttonGetTrainingData.setTextColor( Color.BLACK )
            }
            1 -> {
                buttonGetTrainingData.text = "Below Threshold"
                buttonGetTrainingData.setTextColor( Color.BLUE )
            }
            2 -> trainingExamplesView.setBackgroundColor( Color.argb( 75, 255, 100, 100 ) )
            3 -> {
                buttonGetTrainingData.text = "Collecting Data"
                buttonGetTrainingData.setTextColor( Color.RED )
            }
             5 -> {
                 // TODO: remove this? Animate?
                 buttonGetTrainingData.text = "Added Data"
                 buttonGetTrainingData.setTextColor( Color.GREEN )
             }
        }
    }

    override fun onCreate( savedInstanceState: Bundle? ) {
        super.onCreate(savedInstanceState)
        //retainInstance = true

        val simpleCallback = object : SwipeToDelete( context )
        {
            override fun onSwiped(viewHolder: RecyclerView.ViewHolder?, direction: Int)
            {
                val position = viewHolder!!.adapterPosition //get position which is swipe
                stateManager.removeTrainingSeriesObject( indexOfGesture, position )
            }
        }
        itemTouchHelper = ItemTouchHelper( simpleCallback )
    }

    override fun onStart(  ) {
        super.onStart(  )
        textEdit.setOnEditorActionListener( { _, _, _ ->
            stateManager.setNameOfGesture( indexOfGesture, textEdit.text.toString(  ) )
            val imm = context.getSystemService( Context.INPUT_METHOD_SERVICE ) as InputMethodManager
            imm.hideSoftInputFromWindow( view?.windowToken, 0 )
            textEdit.clearFocus(  )

            false
        })
        textEdit.onFocusChangeListener = OnFocusChangeListener { _, hasFocus ->
            if ( !hasFocus )
                stateManager.setNameOfGesture( indexOfGesture, textEdit.text.toString(  ) )
        }

        constraintLayout.setOnClickListener( {
            val imm = context.getSystemService( Context.INPUT_METHOD_SERVICE ) as InputMethodManager
            imm.hideSoftInputFromWindow( view?.windowToken, 0 )
        } )

        switch.setOnClickListener {
            switch.requestFocus(  )
            stateManager.setGestureActive( indexOfGesture, switch.isChecked )
        }

        buttonGetTrainingData.setOnClickListener {
            stateManager.waitForExampleData( indexOfGesture )
        }

        buttonClearAllTrainingData.setOnClickListener {
            stateManager.clearAllExampleData( indexOfGesture )
        }
        Log.d(debugTag,"onStart" )
    }

    override fun onStop(  ) {
        super.onStop(  )
        switch.isChecked = false
        stateManager.waitForExampleData( -1 ) // Stop recording of example if that was happening
        Log.d(debugTag,"onStop" )
    }

    override fun onCreateView( inflater: LayoutInflater?, container: ViewGroup?,
                                savedInstanceState: Bundle? ): View? {

        val view : View = inflater!!.inflate( R.layout.fragment_gesture_editor, container, false )
        scrollView = view.scrollView_EditGesture
        constraintLayout = view.constrain_Background
        textEdit = view.textInput_gestureName
        switch = view.switch_gestureActive
        numExamples = view.textView_gestureExamples
        buttonGetTrainingData = view.button_collectTrainingData
        buttonClearAllTrainingData = view.button_clearAllTrainingData
        trainingExamplesView = view.recyclerVIew_GestureEditor
        val layoutManager = LinearLayoutManager( view.context )
        itemTouchHelper.attachToRecyclerView( trainingExamplesView )
        trainingExamplesView.layoutManager = layoutManager
        //trainingExamplesView.adapter = gesture?.trainingSeriesObjects!!.rAdapter
        trainingExamplesView.addItemDecoration( DividerItemDecoration( trainingExamplesView.context, layoutManager.orientation ) )


        // Fix this
        if ( gesture != null ) {
            textEdit.setText(gesture!!.nameOfGesture)
            switch.isChecked = gesture!!.active
            numExamples.text = gesture!!.numberOfRecordings.toString()
            trainingExamplesView.swapAdapter(gesture!!.trainingSeriesObjects.rAdapter, true)
        }
        //--

        initialized = true
        return view
    }

    fun setName ( name : String )
    {
        handler.post( object : Runnable {
            override fun run() {
                run {
                    if (textEdit.text.toString() != name && !textEdit.hasFocus()) // Stop infinite callback loop
                        textEdit.setText(name)
                }
            }
        } )
    }

    fun setActive( active : Boolean )
    {
        handler.post( object : Runnable {
            override fun run() {
                run {
                    if ( switch.isChecked != active )
                        switch.isChecked = active
                }
            }
        } )
    }

    fun setNumberOfExamples( num : Int )
    {
        handler.post( object : Runnable {
            override fun run() {
                run {
                    numExamples.text = num.toString(  ) // C++ -> Java only, no check needed
                }
            }
        } )
    }

    fun setNewGesture(temporaryGesture : Gesture, gestureIndex : Int ) {
        gesture?.unregisterAllListeners(  ) // Unregister all listeners from old gesture
        temporaryGesture.nameOfGestureListener = { name : String -> setName( name ) }
        temporaryGesture.activeListener = { active : Boolean -> setActive( active ) }
        temporaryGesture.numberOfRecordingsListener = { num : Int -> setNumberOfExamples( num ) }
        handler.post( object : Runnable {
            override fun run(  ) {
                run {
                    indexOfGesture = gestureIndex
                    gesture = temporaryGesture
                    // Initialize
                    if ( initialized )
                    {
                        textEdit.setText(temporaryGesture.nameOfGesture)
                        switch.isChecked = temporaryGesture.active
                        numExamples.text = temporaryGesture.numberOfRecordings.toString()
                        trainingExamplesView.swapAdapter(temporaryGesture.trainingSeriesObjects.rAdapter, true)
                    }
                }
            }
        } )
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
        stateManager.waitForExampleData( -1 ) // Stop recording of example if that was happening
        Log.d(debugTag,"onDestroy" )
    }

    override fun onDestroyView() {
        super.onDestroyView()
        stateManager.waitForExampleData( -1 ) // Stop recording of example if that was happening
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

    override fun onResume() {
        super.onResume(  )
        if ( gesture != null )
        {
            textEdit.setText(gesture!!.nameOfGesture)
            switch.isChecked = gesture!!.active
            numExamples.text = gesture!!.numberOfRecordings.toString()
        }
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
