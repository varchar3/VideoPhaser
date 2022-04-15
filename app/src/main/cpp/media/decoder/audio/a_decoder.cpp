//
// Created by Administrator on 3/21/2022.
//

/**
 * 主要工作：音频解码
 * 重采样 SwrContext 就是改变音频的采样率、sample format、声道数等参数，使之按照我们期望的参数输出
 * */

#include <locale>
#include "a_decoder.h"
#include "../video/v_decoder.h"

AudioDecoder::AudioDecoder(JNIEnv *env, const jstring path, bool forSynthesizer)
        : BaseDecoder(env, path, forSynthesizer) {};

AudioDecoder::~AudioDecoder() {
    if (m_render != nullptr){
        delete m_render;
    }
}

void AudioDecoder::SetRender(AudioRender *r) {
    m_render = r;
}

void AudioDecoder::Prepare(JNIEnv *env) {
    InitSwr();
    InitOutBuffer();
    InitRender();
}

void AudioDecoder::InitSwr() {
    //解码上下文，从父类 BaseDecoder 中获取
    AVCodecContext *codec_ctx = get_codec_context();
    //用来申请一个重采样结构体
    swr_ctx = swr_alloc();

    //输入通道格式--即解码出来的通道格式
    av_opt_set_int(swr_ctx, "in_channel_layout", codec_ctx->channel_layout, 0);
    //输出通道格式--即解码后转换到的通道格式（下同）
    av_opt_set_int(swr_ctx, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);

    //配置输入/输出采样率
    av_opt_set_int(swr_ctx, "in_sample_rate", codec_ctx->sample_rate, 0);
    av_opt_set_int(swr_ctx, "out_sample_rate", GetSampleRate(codec_ctx->sample_rate), 0);

    //配置输入/输出数据格式
    av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", codec_ctx->sample_fmt, 0);
    av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", GetSampleFormat(), 0);
    //也可以通过 swr_alloc_set_opts() 统一设置

    //设置好后通过swr_init初始化swr结构体
    swr_init(swr_ctx);
}

/**
 * 目标采样个数 = 原采样个数 *（目标采样率 / 原采样率）
 * */
void AudioDecoder::InitOutBuffer() {
    // 重采样后一个通道采样数,重采样数会变化,即定期从里边取出m_dest_nb_sample个字节
    // 区分abc的值并分5种方式取整 a * b / c 若无意外 a* 1/1 = a 1024
    m_dest_nb_sample = (int) av_rescale_rnd(
            ACC_NB_SAMPLES,//a
            GetSampleRate(get_codec_context()->sample_rate),//b 目标采样率
            get_codec_context()->sample_rate,//c 原采样率
            AV_ROUND_UP);
    // 获取给定音频参数所需的缓冲区大小。
    m_dest_data_size = (size_t) av_samples_get_buffer_size(
            nullptr,
            ENCODE_AUDIO_DEST_CHANNEL_COUNTS,
            m_dest_nb_sample,
            GetSampleFormat(),
            1);

    m_out_buffer[0] = static_cast<uint8_t *>(malloc(m_dest_data_size));
}

void AudioDecoder::InitRender() {
    m_render->InitRender();
}

void AudioDecoder::Render(AVFrame *oneFrame) {
    // 转换，返回每个通道的样本数 m_dest_data_size为双声道所以得*1/2
    int ret = swr_convert(
            swr_ctx,
            m_out_buffer,
            m_dest_data_size / 2,
            (const uint8_t **) oneFrame->data,
            oneFrame->nb_samples);

    if (ret > 0) {
        m_render->Render(m_out_buffer[0], (size_t) m_dest_data_size);
    }
}

void AudioDecoder::Release() {
    if (swr_ctx != nullptr) {
        swr_free(&swr_ctx);
    }
    if (m_render != nullptr) {
        m_render->ReleaseRender();
    }
    ReleaseOutBuffer();
}

void AudioDecoder::ReleaseOutBuffer() {
    if (m_out_buffer[0] != nullptr) {
        free(m_out_buffer[0]);
        m_out_buffer[0] = nullptr;
    }
}