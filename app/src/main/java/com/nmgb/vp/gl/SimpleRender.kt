package com.nmgb.vp.gl

import android.opengl.GLES30
import android.opengl.GLSurfaceView
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

/**
- @author:  LZC
- @time:  2/7/2022
- @desc:
 */
class SimpleRender(
    private var drawer:IDrawer
) :GLSurfaceView.Renderer {
    override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
        GLES30.glClearColor(0.0f,0.0f,0.0f,0.0f)
        GLES30.glClear(GLES30.GL_COLOR_BUFFER_BIT)
        val texture = IntArray(1)
        GLES30.glGenTextures(1, texture, 0) //生成纹理
        drawer.setTextureID(texture[0])
    }

    override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
        GLES30.glViewport(0,0,width,height)
        drawer.setWorldSize(width,height)
    }

    override fun onDrawFrame(gl: GL10?) {
        drawer.draw()
    }
}