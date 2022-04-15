package com.nmgb.vp.decoder.extractor

import android.media.MediaFormat
import android.util.Log
import java.nio.ByteBuffer

/**
- @author:  LZC
- @time:  1/6/2022
- @desc: sb作者還要轉接這類，我tm直接後期改了他 视频提取器
 */
class VideoExtractor(path:String) : IExtractor {
    private val vExtractor = MMExtractor(path)

    override fun getFormats(): MediaFormat? {
        return vExtractor.getVideoFormatTrack()
    }

    override fun readBuffer(byteBuffer: ByteBuffer): Int {
        return vExtractor.readBuffer(byteBuffer)
    }

    override fun getCurrentTimeStamp(): Long {
        return vExtractor.currentTimeStamp
    }

    override fun seek(pos: Long): Long {
        return vExtractor.seekTo(pos)
    }

    override fun setStartPos(pos: Long) {
        return vExtractor.setStartPosition(pos)
    }

    override fun stop() {
        return vExtractor.stop()
    }
}