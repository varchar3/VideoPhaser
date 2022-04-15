package com.nmgb.vp.activity

import android.Manifest
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Environment
import android.view.Surface
import android.view.SurfaceHolder
import android.widget.Toast
import com.nmgb.vp.databinding.ActivityFfmpegGlplayerBinding
import com.permissionx.guolindev.PermissionX
import java.io.File

class FFmpegGLPlayerActivity : AppCompatActivity() {
    private lateinit var binding:ActivityFfmpegGlplayerBinding
    private var player: Int ?= null
    private var path =  Environment.getExternalStorageDirectory().absolutePath +
            "/android/data/com.tencent.mobileqq/tencent/qqfile_recv/1638185135886.mp4"
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityFfmpegGlplayerBinding.inflate(layoutInflater)
        setContentView(binding.root)
        permissions()
    }

    private fun permissions() {
        PermissionX.init(this)
            .permissions(Manifest.permission.READ_EXTERNAL_STORAGE)
            .request { allGranted, _, _ ->
                if (allGranted) {
                    initFFMPeg()
                } else {
                    Toast.makeText(this, "no permission", Toast.LENGTH_SHORT).show()
                }
            }
    }

    private fun initFFMPeg() {
            binding.sfv.holder.addCallback(object :SurfaceHolder.Callback{
                override fun surfaceCreated(holder: SurfaceHolder) {
                    if (player == null){
                        player = createGLPlayer(path,holder.surface)
                        glPlayOrPause(player!!)
                        Toast.makeText(this@FFmpegGLPlayerActivity,"start play",
                            Toast.LENGTH_SHORT).show()
                    }
                }

                override fun surfaceChanged(
                    holder: SurfaceHolder,
                    format: Int,
                    width: Int,
                    height: Int
                ) {
                }

                override fun surfaceDestroyed(holder: SurfaceHolder) {
                    glStop(player!!)
                }
            })
    }

    private external fun createGLPlayer(path:String,surface:Surface):Int

    private external fun glPlayOrPause(player:Int)

    private external fun glStop(player: Int)

    companion object{
        init {
            System.loadLibrary("native-lib")
        }
    }
}