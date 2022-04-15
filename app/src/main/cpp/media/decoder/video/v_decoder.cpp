//
// Created by Administrator on 3/3/2022.
//
/**
 * 解码主体
 * 本结构体主要是：
 * 1.@InitRender()初始化渲染器
 * 2.@InitBuffers()初始化缓存
 * 3.@InitSws()初始化缩放转换器
 * */

#include "v_decoder.h"
#include "../../../utils/logger.h"
#include <jni.h>

VideoDecoder::VideoDecoder(JNIEnv *env, jstring path, bool for_synthesizer)
        : BaseDecoder(env, path, for_synthesizer) {
}

VideoDecoder::~VideoDecoder() {}

void VideoDecoder::Prepare(JNIEnv *env) {
    InitRender(env);
    InitBuffers();
    InitSws();
}

bool VideoDecoder::NeedLoopDecode() {
    return true;
}

/**
 * 首先的首先!设置渲染器 渲染器继承 VideoRender
 * */
void VideoDecoder::SetRender(VideoRender *render) {
    this->m_video_render = render;
}

/**
 * 一是，将渲染设置给视频解码器；
 * 二是，调用渲染器的 InitRender 方法初始化渲染器，
 * 并获得目标画面宽高 width(), height(),由 m_codec_ctx 提供
 * 最后是，调用渲染器 Render 方法，进行渲染。
 * */
void VideoDecoder::InitRender(JNIEnv *env) {
    if (m_video_render != nullptr) {
        int dst_size[2] = {-1, 1};
        m_video_render->InitRender(env, width(), height(), dst_size);
        m_dst_w = dst_size[0];
        m_dst_h = dst_size[1];
        if (m_dst_w = -1) {
            m_dst_w = width();
        }
        if (m_dst_h = -1) {
            m_dst_h = height();
        }
    } else {
        LOGI("DoneDecode","Init render error, you should call SetRender first!")
    }
}

/**
 * AVFrame 描述了解码后的（原始）音频或视频数据
 * linesize:图像在内存里是按行存储的。扫描行宽度就是存储一行像素，用了多少字节的内存
 * */
void VideoDecoder::InitBuffers() {
    m_rgb_frame = av_frame_alloc();
    //获取缓存大小 a negative error code in case of failure <0
    int numBytes = av_image_get_buffer_size(DST_FORMAT, m_dst_w, m_dst_h, 1);
    //分配内存 对应src 需要*未签名的8位转换成bit
    m_buf_for_rgb_frame = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    //将内存分配给m_rgb_frame，并将内存格式化为3个通道（RGB888）后，分别保存其地址
    av_image_fill_arrays(
            m_rgb_frame->data,
            m_rgb_frame->linesize,
            m_buf_for_rgb_frame,
            DST_FORMAT,
            m_dst_w,
            m_dst_h,
            1);
}

void VideoDecoder::InitSws() {
    //m_sws_context执行的是缩放转换操作
    //前三个为source 的寬、高及PixelFormat，四到六個參數分別代表 destination 的寬、高及PixelFormat
    //第七個參數則代表要使用哪種scale的方法 ，最後三個參數，如無使用，可以都填上nullptr。
    m_sws_context = sws_getContext(width(), height(), video_pixel_format(),
                                   m_dst_w, m_dst_h, DST_FORMAT,
                                   SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
}

void VideoDecoder::Release() {
    if (m_rgb_frame != nullptr) {
        av_frame_free(&m_rgb_frame);
        m_rgb_frame = nullptr;
    }
    if (m_buf_for_rgb_frame != nullptr) {
        free(m_buf_for_rgb_frame);
        m_buf_for_rgb_frame = nullptr;
    }
    if (m_sws_context != nullptr) {
        sws_freeContext(m_sws_context);
        m_sws_context = nullptr;
    }
    if (m_video_render != nullptr) {
        m_video_render->ReleaseRender();
        m_video_render = nullptr;
    }
}

/**
 * 渲染出这一帧
 * */
void VideoDecoder::Render(AVFrame *frame) {
    sws_scale(
            m_sws_context,
            frame->data,
            frame->linesize,
            0,
            height(),
            m_rgb_frame->data,
            m_rgb_frame->linesize);

    OneFrame *one_frame = new OneFrame(
            m_rgb_frame->data[0],
            m_rgb_frame->linesize[0],
            frame->pts,
            av_get_time_base_q(),
            nullptr,
            false);

    m_video_render->Render(one_frame);
}







