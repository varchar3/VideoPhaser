package com.nmgb.vp

import android.Manifest
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Environment
import android.view.Surface
import android.widget.Toast
import com.nmgb.vp.databinding.ActivityMainBinding
import com.nmgb.vp.decoder.*
import com.nmgb.vp.gl.IDrawer
import com.nmgb.vp.gl.SimpleRender
import com.nmgb.vp.gl.GLVideoDrawer
import com.permissionx.guolindev.PermissionX
import java.util.concurrent.ExecutorService
import java.util.concurrent.Executors

class MainActivity : AppCompatActivity(), IDecoderStateListener {
    private lateinit var binding:ActivityMainBinding
    private lateinit var audioDecoder: AudioDecoder
    private lateinit var videoDecoder: VideoDecoder
    private lateinit var drawer: IDrawer

    private val path =  Environment.getExternalStorageDirectory().absolutePath +
            "/android/data/com.tencent.mobileqq/tencent/qqfile_recv/1638185135886.mp4"
    //创建线程池
    private val threadPool: ExecutorService = Executors.newFixedThreadPool(2)

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        permissions()
        binding.sfv.setOnClickListener {
            if (audioDecoder.mState == DecodeState.PAUSE) {
                audioDecoder.go()
                videoDecoder.go()
            }else{
                audioDecoder.pause()
                videoDecoder.pause()
            }
        }
    }

    private fun permissions() {
        PermissionX.init(this)
            .permissions(Manifest.permission.READ_EXTERNAL_STORAGE)
            .request { allGranted, _, _ ->
                if (allGranted) {
                    initRender()
                } else {
                    Toast.makeText(this, "1", Toast.LENGTH_SHORT).show()
                }
            }
    }

    private fun initRender(){
        drawer = GLVideoDrawer()
        drawer.getSurfaceTexture {
            playWithGlSurfaceView(Surface(it))
        }
        binding.sfv.setEGLContextClientVersion(2)
        binding.sfv.setRenderer(SimpleRender(drawer))
    }

    /**
     * 用到的是openglEs来渲染
     * */
    private fun playWithGlSurfaceView(sf: Surface){
        //创建视频解码器
        videoDecoder = VideoDecoder(path, null, sf)

        drawer.setVideoSize(1920, 1080)

        threadPool.execute(videoDecoder)
        //创建音频解码器
        audioDecoder = AudioDecoder(path)
        threadPool.execute(audioDecoder)
        //开启播放
        audioDecoder.go()
        videoDecoder.go()
    }

    private fun playWithSurfaceView(){
        //创建视频解码器,参数2传入binding里的SurfaceView
        videoDecoder = VideoDecoder(path, null, null)
        threadPool.execute(videoDecoder)
        //创建音频解码器
        audioDecoder = AudioDecoder(path)
        threadPool.execute(audioDecoder)
        //开启播放
        audioDecoder.go()
        videoDecoder.go()
    }



    override fun decodePrepare(decoder: IDecoder) {
    }

    override fun decodeStart(decoder: IDecoder) {
    }

    override fun decoderRunning(decoder: IDecoder) {
    }

    override fun decodePause(decoder: IDecoder) {
    }

    override fun decodeError(decoder: IDecoder, msg: String) {
    }

    override fun decodeFinish(decoder: IDecoder) {
    }

    override fun decodeStop(decoder: IDecoder) {
    }

    override fun decoderDestroy(decoder: IDecoder) {
    }

}