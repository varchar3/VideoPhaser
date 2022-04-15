package com.nmgb.vp.decoder

import android.media.MediaFormat

/**
- @author:  LZC
- @time:  1/5/2022
- @desc:
 */
interface IDecoder :Runnable{

    fun go()

    fun pause()

    fun stop()

    fun seekTo(pos: Long): Long

    fun seekAndPlay(pos: Long): Long

    fun isDecoding():Boolean

    fun isSeeking():Boolean

    fun isStop():Boolean

    fun getWidth():Int

    fun getHeight():Int

    /**
     * 获取视频长度
     */
    fun getDuration():Long

    /**
     * 获取视频旋转角度
     */
    fun getRotationAngle(): Int

    /**
     * 获取音视频对应的格式参数
     */
    fun getMediaFormat(): MediaFormat?

    /**
     * 获取音视频对应的媒体轨道
     */
    fun getTrack(): Int

    /**
     * 获取解码的文件路径
     */
    fun getFilePath(): String

    fun getCurrentTimeStamp(): Long

}