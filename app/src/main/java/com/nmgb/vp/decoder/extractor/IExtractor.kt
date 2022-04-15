package com.nmgb.vp.decoder.extractor

import android.media.MediaFormat
import java.nio.ByteBuffer

/**
- @author:  LZC
- @time:  1/5/2022
- @desc: 音视频抽取器，获取源源不断的音视频数据
 */
interface IExtractor {
    /**
     * 获取音视频格式参数
     */
    fun getFormats(): MediaFormat?

    /**
     * 读取音视频数据
     */
    fun readBuffer(byteBuffer : ByteBuffer):Int

    /**
     * 获取当前帧时间
     */
    fun getCurrentTimeStamp(): Long

    /**
     * Seek到指定位置，并返回实际帧的时间戳
     */
    fun seek(pos: Long): Long

    fun setStartPos(pos: Long)

    /**
     * 停止读取数据
     */
    fun stop()
}