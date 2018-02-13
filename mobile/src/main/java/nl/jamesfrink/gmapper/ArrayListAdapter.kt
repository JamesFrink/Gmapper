package nl.jamesfrink.gmapper

import android.os.Handler
import android.support.v7.widget.RecyclerView

/**
 * Created by james on 25/01/2018.
 */


class ArrayListAdapter < T >  (  )
{   // this container / class is to make C++ -> Java interaction less cluttered

    companion object {
        var handler = Handler( )
    }

    private var arrayList = ArrayList< T > (  )
    lateinit var rAdapter : RecyclerView.Adapter< * >
    var size  = arrayList.size

    fun setAdapter( adapter : RecyclerView.Adapter< * > )
    {
        rAdapter = adapter
    }

    fun add ( element : T ) : Boolean
    {
        var success = arrayList.add( element )
        handler.post( object : Runnable {
            override fun run() {
                run {
                    rAdapter.notifyItemInserted(arrayList.size - 1)
                    size = arrayList.size
                }
            }
        } )
        return success
    }

    operator fun get ( index : Int ) : T
    {
        return arrayList[ index ]
    }

    fun set ( index : Int, element : T ) : T
    {
        var el = arrayList.set( index, element )
        handler.post( object : Runnable {
            override fun run() {
                run {
                    rAdapter.notifyItemChanged( index )
                }
            }
        } )
        return el
    }

    fun update ( index : Int )
    {
        handler.post( object : Runnable {
            override fun run() {
                run {
                    rAdapter.notifyItemChanged( index )
                }
            }
        } )
    }

    fun remove ( index : Int ) : T
    {
        var el = arrayList.removeAt( index )
        handler.post( object : Runnable {
            override fun run() {
                run {
                    rAdapter.notifyItemRemoved( index )
                    size = arrayList.size
                }
            }
        } )
        return el
    }

    fun size (  ) : Int
    {
        return arrayList.size
    }

    fun clear(  )
    {
        val oldSize = arrayList.size
        handler.post( object : Runnable {
            override fun run() {
                run {
                    arrayList.clear(  )
                    size = arrayList.size
                    rAdapter.notifyItemRangeRemoved( 0, oldSize )
                }
            }
        } )
    }
}