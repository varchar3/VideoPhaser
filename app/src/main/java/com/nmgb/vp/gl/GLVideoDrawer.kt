package com.nmgb.vp.gl

import android.graphics.SurfaceTexture
import android.opengl.GLES11Ext
import android.opengl.GLES20
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.nio.FloatBuffer

/**
- @author:  LZC
- @time:  1/26/2022
- @desc: OpenGl运行只允许在一条线程上，否则出错
 */
class GLVideoDrawer : IDrawer {
    /*顶点vertex,相对GL内坐标*/
    private var v = floatArrayOf(
        -1f, -1f,
        1f, -1f,
        -1f, 1f,
        1f, 1f
    )

    /*纹理coors,相对Android内坐标*/
    private var c = floatArrayOf(
        0f, 1f,
        1f, 1f,
        0f, 0f,
        1f, 0f
    )

    /*纹理id*/
    private var textureId = -1
    /*程序id*/
    private var programId = -1
    /*顶点坐标接收器*/
    private var vertexPositionHandler = -1
    /*纹理坐标接收器*/
    private var texturePositionHandler = -1
    /*纹理接收器*/
    private var textureHandler = -1

    private lateinit var mVertexBuffer:FloatBuffer//顶点缓冲
    private lateinit var mTextureBuffer:FloatBuffer//纹理缓冲

    private var mSurfaceTexture: SurfaceTexture? = null
    private var mSftCb: ((SurfaceTexture)->Unit)? =null

    private var mMatrixHandler = -1
    private var mMatrix: FloatArray? = null
    private var mWorldWidth: Int = -1
    private var mWorldHeight: Int = -1
    private var mVideoWidth: Int = -1
    private var mVideoHeight: Int = -1

    init {
        initPos()
    }

    /*第一件事：初始化顶点*/
    private fun initPos() {
        //float = 4 byte
        val vb = ByteBuffer.allocateDirect(v.size * 4)
        //从小到大
        vb.order(ByteOrder.nativeOrder())
        mVertexBuffer = vb.asFloatBuffer()
        mVertexBuffer.put(v)
        mVertexBuffer.position(0)

        val cb = ByteBuffer.allocateDirect(c.size * 4)
        cb.order(ByteOrder.nativeOrder())
        mTextureBuffer = cb.asFloatBuffer()
        mTextureBuffer.put(c)
        mTextureBuffer.position(0)
    }

    override fun draw() {
        if (textureId != -1) {
            //【步骤1: 适配宽高】
            initMatrices()
            //【步骤2: 创建、编译并启动OpenGL着色器】
            createGLPrg()
            //【步骤3: 激活并绑定纹理单元】
            activateTexture()
            //【步骤4: 绑定图片到纹理单元】mSurfaceTexture?.updateTexImage()
            updateTexture()
            //【步骤5: 开始渲染绘制】
            doDraw()
        }
    }

    /**
     * 这里分为
     * 1.启用句柄 glEnableVertexAttribArray
     * 2.设置着色器参数 glVertexAttribPointer
     * 分别调用两次，一次给顶点，一次给纹理
     * 3.再绘制数组 glDrawArrays
     * */
    private fun doDraw() {
        //启用顶点句柄
        GLES20.glEnableVertexAttribArray(vertexPositionHandler)
        //启用纹理句柄
        GLES20.glEnableVertexAttribArray(texturePositionHandler)
        //设置着色器参数， 第二个参数表示一个顶点包含的数据数量，这里为xy，所以为2
        GLES20.glVertexAttribPointer(vertexPositionHandler,
            2,GLES20.GL_FLOAT,false,0,mVertexBuffer)
        GLES20.glVertexAttribPointer(texturePositionHandler,
            2,GLES20.GL_FLOAT,false,0,mTextureBuffer)
        //开始绘制 glDrawArrays
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4)
    }

    private fun updateTexture() {
        mSurfaceTexture?.updateTexImage()
    }

    private fun activateTexture() {
        //1.先激活,默认0
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0)
        /**
         *  marking this steps!!!!
         * */
        //2.绑定id到纹理单元
        GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,textureId)
        //新增3: 将变换矩阵传递给顶点着色器
        GLES20.glUniformMatrix4fv(mMatrixHandler, 1, false, mMatrix, 0)
        //将激活的纹理单元传递到着色器里面
        GLES20.glUniform1i(textureHandler, 0)
        //边缘过滤
        GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
            GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR.toFloat())
        GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
            GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR.toFloat())
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
            GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE)
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
            GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE)
    }

    private fun createGLPrg() {
        if (programId == -1) {
            val vertexShader = loadShader(GLES20.GL_VERTEX_SHADER,getVertexShader())
            val fragmentShader = loadShader(GLES20.GL_FRAGMENT_SHADER,getFragmentShader())
            //1.创建
            programId = GLES20.glCreateProgram()
            //2.附着
            GLES20.glAttachShader(programId,vertexShader)
            GLES20.glAttachShader(programId,fragmentShader)
            //3.链接
            GLES20.glLinkProgram(programId)
            //4.获取 接收器
            vertexPositionHandler = GLES20.glGetAttribLocation(programId,"aPosition")
            texturePositionHandler = GLES20.glGetAttribLocation(programId,"aCoordinate")
            textureHandler = GLES20.glGetUniformLocation(programId,"uTexture")
            mMatrixHandler = GLES20.glGetUniformLocation(programId,"uMatrix")
        }
        //5.----------使用OpenGL程序----------
        GLES20.glUseProgram(programId)
    }

    private fun initMatrices() {
        if (mMatrix != null) return
        if (mVideoWidth != -1 && mVideoHeight != -1 &&
            mWorldWidth != -1 && mWorldHeight != -1) {
            val originRatio = mVideoWidth/mVideoHeight.toFloat()//1:2
            val worldRatio = mWorldWidth/mWorldHeight.toFloat()//2:1
            mMatrix = FloatArray(16) //4*4矩阵
            val prjMatrix = FloatArray(16)
            if (mWorldWidth > mWorldHeight){
                if (originRatio > worldRatio){
                    val actualRatio = originRatio/worldRatio
                    //投影矩阵转换
                    android.opengl.Matrix.orthoM(
                        mMatrix,0,
                        -1f,1f,
                        -actualRatio, actualRatio,
                        -1f,3f
                    )
                }else{
                    // 原始比例小于窗口比例，缩放高度度会导致高度超出，因此，高度以窗口为准，缩放宽度
                    val actualRatio = worldRatio/originRatio
                    //投影矩阵转换
                    android.opengl.Matrix.orthoM(
                        mMatrix,0,
                        -actualRatio, actualRatio,
                        -1f,1f,
                        -1f,3f
                    )
                }
            }else {
                if (originRatio > worldRatio){
                    val actualRatio = originRatio/worldRatio
                    android.opengl.Matrix.orthoM(
                        mMatrix,0,
                        -1f,1f,
                        -actualRatio, actualRatio,
                        -1f,3f
                    )
                }else{
                    // 原始比例小于窗口比例，缩放高度会导致高度超出，因此，高度以窗口为准，缩放宽度
                    val actualRatio = worldRatio/originRatio
                    android.opengl.Matrix.orthoM(
                        mMatrix,0,
                        -actualRatio, actualRatio,
                        -1f,1f,
                        -1f,3f
                    )
                }
            }
        }
    }

    override fun setTextureID(id: Int) {
        textureId = id
        mSurfaceTexture = SurfaceTexture(id)
        mSftCb?.invoke(mSurfaceTexture!!)
    }

    override fun getSurfaceTexture(cb: (st: SurfaceTexture) -> Unit) {
        mSftCb = cb
    }

    override fun release() {
        GLES20.glDisableVertexAttribArray(vertexPositionHandler)
        GLES20.glDisableVertexAttribArray(texturePositionHandler)
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0)
        GLES20.glDeleteTextures(1, intArrayOf(textureId), 0)
        GLES20.glDeleteProgram(programId)
    }

    /**
     * 顶点着色器
     * */
    private fun getVertexShader(): String {
                //顶点坐标
        return "attribute vec4 aPosition;" +
                //纹理坐标
                "attribute vec2 aCoordinate;" +
                //用于传递纹理坐标给片元着色器，命名和片元着色器中的一致
                "varying vec2 vCoordinate;" +
                "uniform mat4 uMatrix;"+
                "void main() {" +
                "    gl_Position = aPosition*uMatrix;" +
                "    vCoordinate = aCoordinate;" +
                "}"
    }

    /**
     * 片元着色器
     * */
    private fun getFragmentShader(): String {
                //一定要加换行"\n"，否则会和下一行的precision混在一起，导致编译出错
        return "#extension GL_OES_EGL_image_external : require\n" +
                //配置float精度，使用了float数据一定要配置：lowp(低)/mediump(中)/highp(高)
                "precision mediump float;" +
                //从顶点着色器传递进来的纹理坐标
                "varying vec2 vCoordinate;" +
                //从Java传递进入来的纹理单元samplerExternalOES是相机用到
                //uniform sampler2D uTexture;
                "uniform samplerExternalOES uTexture;" +
                "void main() {" +
                //根据纹理坐标，从纹理单元中取色
                "  gl_FragColor=texture2D(uTexture, vCoordinate);" +
                "}"
    }

    private fun loadShader(type: Int, shaderCode: String): Int {
        //根据type创建顶点着色器或者片元着色器
        val shader = GLES20.glCreateShader(type)
        //将资源加入到着色器中，并编译
        GLES20.glShaderSource(shader, shaderCode)
        GLES20.glCompileShader(shader)
        return shader
    }

    override fun setVideoSize(videoW: Int, videoH: Int) {
        mVideoWidth = videoW
        mVideoHeight = videoH
    }

    override fun setWorldSize(worldW: Int, worldH: Int) {
        mWorldWidth = worldW
        mWorldHeight = worldH
    }
}