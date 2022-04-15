package com.nmgb.vp.activity

import android.Manifest
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Environment
import android.util.Log
import android.view.Surface
import android.view.SurfaceHolder
import android.widget.Toast
import com.nmgb.vp.databinding.ActivitySecondBinding
import com.permissionx.guolindev.PermissionX

class SecondActivity : AppCompatActivity() {
    private lateinit var binding: ActivitySecondBinding
    private var player: Int ?= null
    private var path: String ?= null
    private var state: Int = 0
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivitySecondBinding.inflate(layoutInflater)
        setContentView(binding.root)
        path = Environment.getExternalStorageDirectory().absolutePath +
                "/android/data/com.tencent.mobileqq/tencent/qqfile_recv/1638185135886.mp4"
        permissions()
    }

    private fun permissions() {
        PermissionX.init(this)
            .permissions(Manifest.permission.READ_EXTERNAL_STORAGE)
            .request { allGranted, _, _ ->
                if (allGranted) {
                    initFFMPeg()
                } else {
                    Toast.makeText(this, "1", Toast.LENGTH_SHORT).show()
                }
            }
    }

    private fun initFFMPeg() {
        binding.sfv.holder.addCallback(object :SurfaceHolder.Callback{
            override fun surfaceCreated(holder: SurfaceHolder) {
                if (player == null) {
                    player = createPlayer(path!!, holder.surface)
                    state = play(player!!)
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
            }
        })

        binding.sfv.setOnClickListener {
            state = if (state == 1){
                pause(player!!)
            }else{
                play(player!!)
            }
        }

        binding.tv.text = stringFormJNI()
    }
    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    private external fun stringFormJNI():String

    private external fun createPlayer(path: String, surface: Surface): Int

    private external fun play(player: Int):Int

    private external fun pause(player: Int):Int

    companion object{
        // Used to load the 'native-lib' library on application startup.
        init {
            System.loadLibrary("native-lib")
        }
    }
}