package com.nmgb.vp.decoder

/**
- @author:  LZC
- @time:  1/5/2022
- @desc:
 */
interface IDecoderStateListener {
    fun decodePrepare(decoder:IDecoder)
    fun decodeStart(decoder:IDecoder)
    fun decoderRunning(decoder:IDecoder)
    fun decodePause(decoder:IDecoder)
    fun decodeError(decoder:IDecoder,msg:String)
    fun decodeFinish(decoder:IDecoder)
    fun decodeStop(decoder:IDecoder)
    fun decoderDestroy(decoder:IDecoder)
}