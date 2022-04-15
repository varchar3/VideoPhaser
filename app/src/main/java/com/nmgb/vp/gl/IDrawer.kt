package com.nmgb.vp.gl

import android.graphics.SurfaceTexture


/**
- @author:  LZC
- @time:  1/26/2022
- @desc:
 */
interface IDrawer {
    fun draw()
    fun setTextureID(id:Int)
    fun release()
    //设置视频的原始宽高
    fun setVideoSize(videoW: Int, videoH: Int)
    //设置OpenGL窗口宽高
    fun setWorldSize(worldW: Int, worldH: Int)
    //用于提供SurfaceTexture
    fun getSurfaceTexture(cb:(st: SurfaceTexture)->Unit) {}
}