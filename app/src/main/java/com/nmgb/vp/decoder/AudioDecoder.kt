package com.nmgb.vp.decoder

import android.media.*
import android.provider.Contacts.SettingsColumns.KEY
import android.provider.MediaStore
import android.util.SparseArray
import com.nmgb.vp.decoder.extractor.AudioExtractor
import com.nmgb.vp.decoder.extractor.IExtractor
import java.lang.Integer.getInteger
import java.nio.ByteBuffer
import android.media.AudioAttributes

import android.media.AudioTrack

/**
- @author:  LZC
- @time:  1/18/2022
- @desc: 音频播放器
 */
class AudioDecoder(path: String) :BaseDecoder(path){
    /*采样率*/
    private var sampleRate = -1
    /*声音通道数 MONO单 STEREO立体声*/
    private var channels = 1
    /*PCM采样数,即深度*/
    private var bit = AudioFormat.ENCODING_PCM_16BIT
    /*音乐播放器*/
    private var audioTrack:AudioTrack? = null
    /*音频数据存储*/
    private var audioTempBuffer:ShortArray? = null

    override fun render(outputBuffers: ByteBuffer, bufferInfo: MediaCodec.BufferInfo) {
        /**
        * short是2字节,byte是1字节,define the capacity of the outputTempBuffer we needed
        * */
        if(audioTempBuffer!!.size < bufferInfo.size/2) {
            audioTempBuffer = ShortArray(bufferInfo.size/2)
        }
        outputBuffers.position(0)
        //先是把短数据转换并用get()放进audioTempBuffer里边
        outputBuffers.asShortBuffer().get(audioTempBuffer,0,bufferInfo.size/2)
        //再是给到audioTrack存放
        audioTrack?.write(audioTempBuffer!!,0,bufferInfo.size/2)
    }

    override fun doneDecode() {
        audioTrack?.stop()
        audioTrack?.release()
    }

    override fun check(): Boolean {
        return true
    }

    override fun initExtractor(path: String): IExtractor {
        return AudioExtractor(path)
    }

    override fun initSpecParams(format: MediaFormat) {
        try {
            channels = format.getInteger(MediaFormat.KEY_CHANNEL_COUNT)
            sampleRate = format.getInteger(MediaFormat.KEY_SAMPLE_RATE)
            //默认 AudioFormat.ENCODING_PCM_16BIT
            if (format.containsKey(MediaFormat.KEY_PCM_ENCODING)) {
                bit = format.getInteger(MediaFormat.KEY_PCM_ENCODING)
            }
        }catch (e: Exception){
            print(e)
        }
    }

    override fun configCodec(codec: MediaCodec, format: MediaFormat): Boolean {
        //音频不需要surface，直接传null
        codec.configure(format,null,null,0)
        return true
    }

    override fun initRender(): Boolean {
        val channel = if (this.channels == 1) {
            AudioFormat.CHANNEL_OUT_MONO
        }else{
            AudioFormat.CHANNEL_OUT_STEREO
        }

        /**
         * (Rate * bit * 1 )/8 * channel = bit per PCM
         * */
        val minBufferSize = AudioTrack.getMinBufferSize(sampleRate,channel,bit)

        audioTempBuffer = ShortArray(minBufferSize/2)

        audioTrack = AudioTrack(
            AudioManager.STREAM_MUSIC,//播放类型：音乐
            sampleRate, //采样率
            channel, //通道
            bit, //采样位数
            minBufferSize, //缓冲区大小
            AudioTrack.MODE_STREAM) //播放模式：数据流动态写入，另一种是一次性写入
        audioTrack!!.play()
        return true
    }


}