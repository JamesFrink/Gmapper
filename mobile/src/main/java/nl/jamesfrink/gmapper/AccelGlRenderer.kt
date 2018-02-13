package nl.jamesfrink.gmapper

import android.opengl.GLES20
import android.opengl.GLES30
import android.opengl.GLSurfaceView
import android.opengl.Matrix
import android.os.SystemClock
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10
import java.nio.FloatBuffer
import java.nio.ByteBuffer
import java.nio.ByteOrder




/**
 * Created by james on 31/01/2018.
 */

fun loadShader(type: Int, shaderCode: String): Int {

    // create a vertex shader type (GLES20.GL_VERTEX_SHADER)
    // or a fragment shader type (GLES20.GL_FRAGMENT_SHADER)
    val shader = GLES20.glCreateShader( type )

    // add the source code to the shader and compile it
    GLES20.glShaderSource(shader, shaderCode)
    GLES20.glCompileShader(shader)

    return shader
}

 class Triangle {

     private val vertexShaderCode = "uniform mat4 uMVPMatrix;" +
             "attribute vec4 vPosition;" +
             "void main() {" +
// the matrix must be included as a modifier of gl_Position
// Note that the uMVPMatrix factor *must be first* in order
// for the matrix multiplication product to be correct.
             "  gl_Position = uMVPMatrix * vPosition;" +
             "}"

     private val fragmentShaderCode = (
             "precision mediump float;" +
                     "uniform vec4 vColor;" +
                     "void main() {" +
                     "  gl_FragColor = vColor;" +
                     "}")

     private val COORDS_PER_VERTEX = 3
     private var triangleCoords = floatArrayOf(
             // in counterclockwise order:
             0.0f, 0.5f, 0.0f, // top
             -0.5f, -0.5f, 0.0f, // bottom left
             0.5f, -0.5f, 0.0f,    // bottom right
             0.0f, 0.5f, 0.0f
     )

     private val vertexBuffer: FloatBuffer
     private val mProgram: Int
     private var mPositionHandle: Int = 0
     private var mColorHandle: Int = 0
     private var mMVPMatrixHandle: Int = 0
     private val vertexCount = triangleCoords.size / COORDS_PER_VERTEX
     private val vertexStride = COORDS_PER_VERTEX * 4 // 4 bytes per vertex

     internal var color = floatArrayOf(0.63671875f, 0.76953125f, 0.22265625f, 0.0f)

     /**
      * Sets up the drawing object data for use in an OpenGL ES context.
      */
     init {
         // initialize vertex byte buffer for shape coordinates
         val bb = ByteBuffer.allocateDirect(
                 // (number of coordinate values * 4 bytes per float)
                 triangleCoords.size * 4)
         // use the device hardware's native byte order
         bb.order(ByteOrder.nativeOrder())

         // create a floating point buffer from the ByteBuffer
         vertexBuffer = bb.asFloatBuffer()
         // add the coordinates to the FloatBuffer
         vertexBuffer.put(triangleCoords)
         // set the buffer to read the first coordinate
         vertexBuffer.position(0)

         // prepare shaders and OpenGL program
         val vertexShader = loadShader(
                 GLES20.GL_VERTEX_SHADER, vertexShaderCode)
         val fragmentShader = loadShader(
                 GLES20.GL_FRAGMENT_SHADER, fragmentShaderCode)

         mProgram = GLES20.glCreateProgram()             // create empty OpenGL Program
         GLES20.glAttachShader(mProgram, vertexShader)   // add the vertex shader to program
         GLES20.glAttachShader(mProgram, fragmentShader) // add the fragment shader to program
         GLES20.glLinkProgram(mProgram)                  // create OpenGL program executables

     }

     /**
      * Encapsulates the OpenGL ES instructions for drawing this shape.
      *
      * @param mvpMatrix - The Model View Project matrix in which to draw
      * this shape.
      */
     fun draw(mvpMatrix: FloatArray) {
         // Add program to OpenGL environment
         GLES20.glUseProgram(mProgram)

         // get handle to vertex shader's vPosition member
         mPositionHandle = GLES20.glGetAttribLocation(mProgram, "vPosition")

         // Enable a handle to the triangle vertices
         GLES20.glEnableVertexAttribArray(mPositionHandle)

         // Prepare the triangle coordinate data
         GLES20.glVertexAttribPointer(
                 mPositionHandle, COORDS_PER_VERTEX,
                 GLES20.GL_FLOAT, false,
                 vertexStride, vertexBuffer)

         // get handle to fragment shader's vColor member
         mColorHandle = GLES20.glGetUniformLocation(mProgram, "vColor")

         // Set color for drawing the triangle
         GLES20.glUniform4fv(mColorHandle, 1, color, 0)

         // get handle to shape's transformation matrix
         mMVPMatrixHandle = GLES20.glGetUniformLocation(mProgram, "uMVPMatrix")
         //MyGLRenderer.checkGlError("glGetUniformLocation")

         // Apply the projection and view transformation
         GLES20.glUniformMatrix4fv(mMVPMatrixHandle, 1, false, mvpMatrix, 0)
         //MyGLRenderer.checkGlError("glUniformMatrix4fv")

         // Draw the triangle
         GLES20.glDrawArrays(GLES20.GL_LINE_STRIP, 0, vertexCount)

         // Disable vertex array
         GLES20.glDisableVertexAttribArray(mPositionHandle)
     }

     fun setTriangleTest( x : Float, y : Float, z : Float )
     {
         triangleCoords[ 1 ] = y * 0.5f
         triangleCoords[ 4 ] = x * -0.5f
         triangleCoords[ 7 ] = z * -0.5f
         triangleCoords[ 10 ] = y * 0.5f
         vertexBuffer.clear(  )
         vertexBuffer.put( triangleCoords )
         vertexBuffer.position(0)

     }
 }

class AccelGlRenderer : GLSurfaceView.Renderer
{
    private val TAG = "MyGLRenderer"
    private var mTriangle: Triangle? = null
    //private var mSquare: Square? = null

    // mMVPMatrix is an abbreviation for "Model View Projection Matrix"
    private val mMVPMatrix = FloatArray(16)
    private val mProjectionMatrix = FloatArray(16)
    private val mViewMatrix = FloatArray(16)
    private val mRotationMatrix = FloatArray(16)

    private var mAngle: Float = 0.toFloat()

    override fun onSurfaceCreated(unused: GL10, config: EGLConfig) {

        // Set the background frame color
        GLES20.glClearColor( 0.0f, 0.0f, 0.0f, 1.0f )

        mTriangle = Triangle()
        //mSquare = Square()
    }

    override fun onDrawFrame(unused: GL10) {
        val scratch = FloatArray(16)

        // Draw background color
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT or GLES20.GL_DEPTH_BUFFER_BIT)

        // Set the camera position (View matrix)
        Matrix.setLookAtM(mViewMatrix, 0, 0.0f, 0.0f, -3.0f, 0.0f, 0f, 0f, 0f, 1.0f, 0.0f)

        // Calculate the projection and view transformation
        Matrix.multiplyMM(mMVPMatrix, 0, mProjectionMatrix, 0, mViewMatrix, 0)

        // Draw square
        //mSquare!!.draw(mMVPMatrix)

        // Create a rotation for the triangle

        // Use the following code to generate constant rotation.
        // Leave this code out when using TouchEvents.
        //var time = SystemClock.uptimeMillis() % 4000L;
        //mAngle = 0.090f * ( time.toInt(  ) );

        Matrix.setRotateM(mRotationMatrix, 0, mAngle, 0f, 0f, 1.0f)

        // Combine the rotation matrix with the projection and camera view
        // Note that the mMVPMatrix factor *must be first* in order
        // for the matrix multiplication product to be correct.
        Matrix.multiplyMM( scratch, 0, mMVPMatrix, 0, mRotationMatrix, 0)

        // Draw triangle
        mTriangle!!.draw( scratch )
    }

    override fun onSurfaceChanged(unused: GL10, width: Int, height: Int) {
        // Adjust the viewport based on geometry changes,
        // such as screen rotation
        GLES20.glViewport(0, 0, width, height)

        val ratio = width.toFloat() / height

        // this projection matrix is applied to object coordinates
        // in the onDrawFrame() method
        Matrix.frustumM(mProjectionMatrix, 0, -ratio, ratio, -1f, 1f, 3f, 7f)
    }

    /**
     * Utility method for debugging OpenGL calls. Provide the name of the call
     * just after making it:
     *
     * <pre>
     * mColorHandle = GLES20.glGetUniformLocation(mProgram, "vColor");
     * MyGLRenderer.checkGlError("glGetUniformLocation");</pre>
     *
     * If the operation is not successful, the check throws an error.
     *
     * @param glOperation - Name of the OpenGL call to check.
     */
    /*fun checkGlError(glOperation: String) {
        val error: Int
        while ((error = GLES20.glGetError()) != GLES20.GL_NO_ERROR) {
            Log.e(TAG, glOperation + ": glError " + error)
            throw RuntimeException(glOperation + ": glError " + error)
        }
    }*/

    /**
     * Returns the rotation angle of the triangle shape (mTriangle).
     *
     * @return - A float representing the rotation angle.
     */
    fun getAngle(): Float {
        return mAngle
    }

    /**
     * Sets the rotation angle of the triangle shape (mTriangle).
     */
    fun setAngle( x : Float, y : Float, z : Float ) {
        mTriangle?.setTriangleTest( x, y, z )
    }


}
