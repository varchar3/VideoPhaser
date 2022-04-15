//
// Created by Administrator on 3/21/2022.
//

#ifndef VIDEOPHASER_A_DECODER_H
#define VIDEOPHASER_A_DECODER_H

#include "../base_decoder.h"
#include "../../render/audio/audio_render.h"
#include "../../../utils/const.h"

extern "C" {
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavutil/audio_fifo.h>
}

class AudioDecoder : public BaseDecoder {
private:
    /*-------------成员变量区---------------*/
    const char *TAG = "a_decoder";
    //重采样结构体
    SwrContext *swr_ctx = nullptr;
    //音频渲染器
    AudioRender *m_render = nullptr;
    //输出缓冲 m_out_buffer[1]创建大小为1类型为uint8_t的数组做缓冲区
    uint8_t *m_out_buffer[1] = {nullptr};
    // 重采样后，每个通道包含的采样数
    // acc默认为1024，重采样后可能会变化
    // 8k,48k都是针对pcm原始的采样
    int m_dest_nb_sample = 1024;
    // 重采样以后，一帧数据的大小
    size_t m_dest_data_size = 0;

    /*-------------方法区---------------*/
    /**
     * 初始化转换工具
     * */
    void InitSwr();

    /**
     * 初始化输出缓冲
     * */
    void InitOutBuffer();

    /**
     * 初始化渲染器
     * */
    void InitRender();

    /**
     * 释放缓冲区
     * */
    void ReleaseOutBuffer();

    /**
     * 采样格式:16位
     * */
    AVSampleFormat GetSampleFormat() {
        if (ForSynthesizer()) {
            return ENCODE_AUDIO_DEST_FORMAT;
        } else {
            return AV_SAMPLE_FMT_S16;
        }
    }

    /**
     * 采样率
     */
    int GetSampleRate(int spr) {
        if (ForSynthesizer()) {
            return ENCODE_AUDIO_DEST_SAMPLE_RATE;//44100Hz
        } else {
            return spr;
        }
    }

public:
    AudioDecoder(JNIEnv *env, const jstring path, bool forSynthesizer);

    ~AudioDecoder();

    void SetRender(AudioRender *r);

protected:
    void Prepare(JNIEnv *env) override;

    void Render(AVFrame *oneFrame) override;

    void Release();

    bool NeedLoopDecode() {
        return true;
    }

    AVMediaType GetMediaType() override {
        return AVMEDIA_TYPE_AUDIO;
    }

};


#endif //VIDEOPHASER_A_DECODER_H
