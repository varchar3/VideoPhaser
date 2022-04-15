package com.nmgb.vp.decoder.extractor

import android.media.MediaFormat
import android.util.Log
import java.nio.ByteBuffer

/**
- @author:  LZC
- @time:  1/6/2022
- @desc:音频提取器
 */
class AudioExtractor(path:String) : IExtractor {
    private var aExtractor = MMExtractor(path)

    override fun getFormats(): MediaFormat {
        return aExtractor.getAudioFormatTrack()!!
    }

    override fun readBuffer(byteBuffer: ByteBuffer): Int {
        return aExtractor.readBuffer(byteBuffer)
    }

    override fun getCurrentTimeStamp(): Long {
        return aExtractor.currentTimeStamp
    }

    override fun seek(pos: Long): Long {
        return aExtractor.seekTo(pos)
    }

    override fun setStartPos(pos: Long) {
        return aExtractor.setStartPosition(pos)
    }

    override fun stop() {
        return aExtractor.stop()
    }
}