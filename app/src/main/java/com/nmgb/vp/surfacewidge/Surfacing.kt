package com.nmgb.vp.surfacewidge

import android.content.Context
import android.util.AttributeSet
import android.view.SurfaceHolder
import android.view.SurfaceView

/**
- @author:  LZC
- @time:  1/10/2022
- @desc: Android 每16ms 发送一个 Vsync 信号 16ms结束后叫下一帧开始处理 bytebuffer，
 交给cpu和gpu处理完显示进频幕上
 过程处理：producers ->bufferQueue -> consumer 不可跨 反之依然
 挖空 phoneWindows一块（设置了透明），用来render此surfaceView，底层混合交给surfaceFlinger

 SurfaceHolder提供 3个方法 创建 改变 销毁
 */
class Surfacing @JvmOverloads constructor(
    context: Context, attrs: AttributeSet? = null
) : SurfaceView(context, attrs),SurfaceHolder.Callback,Runnable {

    init {
        /**
         * holder 是 SurfaceHolder提供的参数
         * */
        holder.addCallback(this)
        //设置一些参数方便后面绘图
        isFocusable = true
        keepScreenOn = true
        isFocusableInTouchMode = true
    }

    override fun run() {
        //子线程循环干某事
    }

    override fun surfaceCreated(p0: SurfaceHolder) {
        Thread(this).start()
    }

    override fun surfaceChanged(p0: SurfaceHolder, p1: Int, p2: Int, p3: Int) {
    }

    override fun surfaceDestroyed(p0: SurfaceHolder) {
    }




}