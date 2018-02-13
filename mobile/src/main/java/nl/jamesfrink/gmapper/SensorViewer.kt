package nl.jamesfrink.gmapper


import android.graphics.Color
import android.os.Bundle
import android.support.v4.app.Fragment
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import com.github.mikephil.charting.charts.LineChart
import com.github.mikephil.charting.components.YAxis
import com.github.mikephil.charting.data.Entry
import com.github.mikephil.charting.data.LineData
import com.github.mikephil.charting.data.LineDataSet
import com.github.mikephil.charting.formatter.IFillFormatter

class SensorViewer : Fragment(  ) {
    lateinit var xChart : LineChart
    lateinit var yChart : LineChart
    lateinit var zChart : LineChart
    val xVals = ArrayList< Entry >(  )
    val yVals = ArrayList< Entry >(  )
    val zVals = ArrayList< Entry >(  )
    lateinit var setXAccel : LineDataSet
    lateinit var setYAccel : LineDataSet
    lateinit var setZAccel : LineDataSet
    lateinit var dataX : LineData
    lateinit var dataY : LineData
    lateinit var dataZ : LineData
    val maxNumData = 1024
    var position = 0
    var initialized = false // Todo de-initialize on stop

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        for ( i in 0 until maxNumData ) {
            xVals.add( Entry( i.toFloat(  ), 0.0f ) )
            yVals.add( Entry( i.toFloat(  ), 0.0f ) )
            zVals.add( Entry( i.toFloat(  ), 0.0f ) )
        }

        setXAccel = LineDataSet( xVals, "X" )
        setXAccel.axisDependency = YAxis.AxisDependency.LEFT
        setXAccel.color = Color.rgb(255, 241, 46)
        setXAccel.setDrawCircles(false)
        setXAccel.lineWidth = 2f
        setXAccel.circleRadius = 3f
        setXAccel.color = Color.rgb(255, 0,  0)
        setXAccel.highLightColor = Color.rgb(244, 0, 117)
        setXAccel.setDrawCircleHole(false)
        setXAccel.fillFormatter = IFillFormatter { dataSet, dataProvider -> xChart.axisLeft.axisMinimum }
        dataX = LineData( setXAccel )
        dataX.setDrawValues( false )

        setYAccel = LineDataSet( yVals, "Y" )
        setYAccel.axisDependency = YAxis.AxisDependency.LEFT
        setYAccel.color = Color.rgb(0, 255,  0)
        setYAccel.setDrawCircles(false)
        setYAccel.lineWidth = 2f
        setYAccel.circleRadius = 3f
        setYAccel.highLightColor = Color.rgb(244, 117, 117)
        setYAccel.setDrawCircleHole(false)
        setYAccel.fillFormatter = IFillFormatter { dataSet, dataProvider -> yChart.axisLeft.axisMinimum }
        dataY = LineData( setYAccel )
        dataY.setDrawValues( false )

        setZAccel = LineDataSet( zVals, "Z" )
        setZAccel.axisDependency = YAxis.AxisDependency.LEFT
        setZAccel.color = Color.rgb(0, 0,  255)
        setZAccel.setDrawCircles(false)
        setZAccel.lineWidth = 2f
        setZAccel.circleRadius = 3f
        setZAccel.highLightColor = Color.rgb(244, 117, 117)
        setZAccel.setDrawCircleHole(false)
        setZAccel.fillFormatter = IFillFormatter { dataSet, dataProvider -> zChart.axisLeft.axisMinimum }
        dataZ = LineData( setZAccel )
        dataZ.setDrawValues( false )
    }

    override fun onCreateView(inflater: LayoutInflater?, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        val view = inflater!!.inflate( R.layout.fragment_sensor_viewer, container, false )
        xChart = view?.findViewById< LineChart >( R.id.chart1 )!!
        yChart = view?.findViewById< LineChart >( R.id.chart2 )!!
        zChart = view?.findViewById< LineChart >( R.id.chart3 )!!
        // set data
        xChart.data = dataX
        xChart.description.isEnabled = false
        yChart.data = dataY
        yChart.description.isEnabled = false
        zChart.data = dataZ
        zChart.description.isEnabled = false
        initialized = true
        return view
    }

    fun addAccelData( x : Float, y : Float, z : Float ) {

        if ( initialized ) {
            xVals[ position ].y = x
            yVals[ position ].y = y
            zVals[ position ].y = z
            setXAccel.notifyDataSetChanged(  )
            setYAccel.notifyDataSetChanged(  )
            setZAccel.notifyDataSetChanged(  )
            dataX.notifyDataChanged(  )
            dataY.notifyDataChanged(  )
            dataZ.notifyDataChanged(  )
            xChart.notifyDataSetChanged(  )
            yChart.notifyDataSetChanged(  )
            zChart.notifyDataSetChanged(  )
            xChart.invalidate(  )
            yChart.invalidate(  )
            zChart.invalidate(  )
            position = ++position % maxNumData
        }
    }

    override fun onViewCreated(view: View?, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
    }

    override fun onStart() {
        super.onStart()
    }

}// Required empty public constructor

