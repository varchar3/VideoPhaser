//
// Created by Administrator on 3/3/2022.
//

#ifndef VIDEOPHASER_V_DECODER_H
#define VIDEOPHASER_V_DECODER_H

#include "../../media/decoder/base_decoder.h"
#include "../../media/render/video/video_render.h"
#include <jni.h>
#include <android/native_window_jni.h>
#include <android/native_window.h>

extern "C" {
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

class VideoDecoder : public BaseDecoder {
private:
    const char *TAG = "VideoDecoder";

    //视频数据目标格式 is RGBA888
    const AVPixelFormat DST_FORMAT = AV_PIX_FMT_RGBA;

    //存放YUV转RGB后的数据
    AVFrame *m_rgb_frame = NULL;

    uint8_t *m_buf_for_rgb_frame = NULL;

    //视频格式转换器
    SwsContext *m_sws_context = NULL;

    //视频渲染器
    VideoRender *m_video_render = NULL;

    //显示的目标宽,高
    int m_dst_w;

    int m_dst_h;

    /**
     * 初始化渲染器
     * */
    void InitRender(JNIEnv *env);

    /**
     * 初始化显示器
     * */
    void InitBuffers();

    /**
     * 初始化视频数据转换器
     * */
    void InitSws();

public:
    VideoDecoder(JNIEnv *env, jstring path, bool for_synthesizer = false);

    ~VideoDecoder();

    void SetRender(VideoRender *render);

protected:
    AVMediaType GetMediaType() override {
        return AVMEDIA_TYPE_VIDEO;
    }

    /**
    * 是否需要循环解码
    */
    bool NeedLoopDecode();

    /**
     * 准备解码环境
     * 在解码线程中回调
     * */
    void Prepare(JNIEnv *env) override;

    /**
     * 渲染，在解码线程中回调
     * */
    void Render(AVFrame *frame) override;

    /**
     * 释放回调
     * */
    void Release() override;

    const char *const LogSpec() {
        return "VIDEO";
    };
};

#endif //VIDEOPHASER_V_DECODER_H
