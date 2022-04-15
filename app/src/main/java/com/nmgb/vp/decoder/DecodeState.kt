package com.nmgb.vp.decoder

/**
- @author:  LZC
- @time:  1/5/2022
- @desc:
 */
enum class DecodeState {
    START,
    DECODING,
    PAUSE,
    SEEKING,
    FINISH,
    STOP,
}