package com.nmgb.vp.decoder

import android.media.MediaCodec
import android.media.MediaFormat
import android.util.Log
import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView
import com.nmgb.vp.decoder.extractor.IExtractor
import com.nmgb.vp.decoder.extractor.VideoExtractor
import java.nio.ByteBuffer
/**
- @author:  LZC
- @time:  1/5/2022
- @desc: surface 和 surfaceView  必须两者有一个不为空 ,
         surface可以从surfaceView的holder获取到
 */
class VideoDecoder(
    path:String,
    surfaceView: SurfaceView?,
    surface:Surface?
) :BaseDecoder(path){

    private val mSurfaceView = surfaceView
    //可理解为装dequeueBuffer的容器
    private var mSurface = surface

    override fun check(): Boolean {
        if (mSurfaceView == null && mSurface == null) {
            Log.i("VideoDecoder","mSurfaceView and mSurface atLeast not null for one")
            mStateListener?.decodeError(this,"surface must not null!")
            return false
        }
        return true
    }

    override fun initExtractor(path: String): IExtractor {
        return VideoExtractor(path)
    }

    override fun initSpecParams(format: MediaFormat) {
        mVideoWidth = format.getInteger(MediaFormat.KEY_WIDTH)
        mVideoHeight = format.getInteger(MediaFormat.KEY_HEIGHT)
        Log.i("alnsldw", "height:$mVideoHeight,width:$mVideoWidth")
    }

    override fun configCodec(codec: MediaCodec, format: MediaFormat): Boolean {
        mVideoWidth = format.getInteger(MediaFormat.KEY_WIDTH)
        mVideoHeight = format.getInteger(MediaFormat.KEY_HEIGHT)
        if (mSurface != null) {
            codec.configure(format, mSurface , null, 0)
            notifyDecode()
        }else if (mSurfaceView?.holder?.surface != null) {
            mSurface = mSurfaceView.holder?.surface
            configCodec(codec, format)
        } else {
            mSurfaceView?.holder?.addCallback(object : SurfaceHolder.Callback2 {
                override fun surfaceRedrawNeeded(holder: SurfaceHolder) {
                }
                override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
                }
                override fun surfaceDestroyed(holder: SurfaceHolder) {
                }
                override fun surfaceCreated(holder: SurfaceHolder) {
                    mSurface = holder.surface
                    configCodec(codec, format)

                }
            })
            return false
        }
        return true
    }

    override fun initRender(): Boolean {
        return true
    }

    override fun render(outputBuffers: ByteBuffer, bufferInfo: MediaCodec.BufferInfo) {
    }

    override fun doneDecode() {
    }
}
