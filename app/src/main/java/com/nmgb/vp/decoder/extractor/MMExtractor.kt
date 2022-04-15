package com.nmgb.vp.decoder.extractor

import android.media.MediaExtractor
import android.media.MediaFormat
import android.util.Log
import java.nio.ByteBuffer

/**
- @author:  LZC
- @time:  1/6/2022
- @desc:
 */
class MMExtractor(path:String?) {

    /*分離器*/
    private var mExtractor: MediaExtractor? = null
    /*音頻通道索引*/
    var mAudioTrack:Int = -1
    /*視頻通道索引*/
    var mVideoTrack:Int = -1
    /*當前時間戳*/
    var currentTimeStamp:Long = 0
    /*開始解碼時間點*/
    private var mStartPosition:Long = 0

    init {
        mExtractor = MediaExtractor()
        mExtractor!!.setDataSource(path!!)
    }

    /**獲取視頻軌道*/
    fun getVideoFormatTrack(): MediaFormat? {
        //【2.1，获取视频多媒体格式】
        for (j in 0 until mExtractor!!.trackCount){
            if (mExtractor!!
                    .getTrackFormat(j)
                    .getString(MediaFormat.KEY_MIME)!!
                    .startsWith("video/")){
                mVideoTrack = j
                break
            }
        }
        return if (mVideoTrack >= 0) mExtractor!!.getTrackFormat(mVideoTrack) else null
    }

    /**獲取音頻軌道*/
    fun getAudioFormatTrack(): MediaFormat? {
        //【2.2，获取音频频多媒体格式】
        for (i in 0 until mExtractor!!.trackCount) {
            if (mExtractor!!
                    .getTrackFormat(i)
                    .getString(MediaFormat.KEY_MIME)!!
                    .startsWith("audio/")){
                mAudioTrack = i
                break
            }
        }
        return if (mAudioTrack >= 0) mExtractor!!.getTrackFormat(mAudioTrack) else null
    }


    /**
     * 讀數據,暴露的方法
     * 解码器传进来的byteBuffer,用于存放待解码数据的缓冲区
     * */
    fun readBuffer(byteBuffer:ByteBuffer):Int {
        //清一下緩衝區
        byteBuffer.clear()
        //選擇通道 根据当前选择的通道（同时只选择一个音/视频通道）
        selectSourceTrack()
        //提取數據
        val readSampleCount = mExtractor!!.readSampleData(byteBuffer,0)

        if (readSampleCount<0){
            return -1
        }
        //记录当前帧的时间戳
        currentTimeStamp = mExtractor!!.sampleTime
        //进入下一帧
        mExtractor!!.advance()

        return readSampleCount
    }

    /**
     * 选择通道
     */
    private fun selectSourceTrack() {
        if (mVideoTrack >= 0) {
            mExtractor!!.selectTrack(mVideoTrack)
        } else if (mAudioTrack >= 0) {
            mExtractor!!.selectTrack(mAudioTrack)
        }
    }

    /**
     * 说明：seek(pos: Long)方法，主要用于跳播，快速将数据定位到指定的播放位置，但是，
     * 由于视频中，除了I帧以外，PB帧都需要依赖其他的帧进行解码，所以，通常只能seek到I帧，
     * 但是I帧通常和指定的播放位置有一定误差，因此需要指定seek靠近哪个关键帧，有以下三种类型：
       SEEK_TO_PREVIOUS_SYNC：跳播位置的上一个关键帧
       SEEK_TO_NEXT_SYNC：跳播位置的下一个关键帧
       SEEK_TO_CLOSEST_SYNC：距离跳播位置的最近的关键帧
     */
    fun seekTo(pos: Long):Long{
        mExtractor!!.seekTo(pos,MediaExtractor.SEEK_TO_PREVIOUS_SYNC)
        return mExtractor!!.sampleTime
    }

    /**
     * 停止讀取數據加釋放資源,客户端退出解码的时候，需要调用stop是否提取器相关资源。
     * */
    fun stop(){
        mExtractor!!.release()
        mExtractor = null
    }

    fun getVideoTrack():Int = mVideoTrack
    fun getAudioTrack():Int = mAudioTrack
    fun getTimeStamp():Long = currentTimeStamp
    fun setStartPosition(position:Long){mStartPosition = position}

}