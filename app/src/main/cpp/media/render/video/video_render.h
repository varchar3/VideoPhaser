//
// Created by Administrator on 3/4/2022.
//

#ifndef VIDEOPHASER_VIDEO_RENDER_H
#define VIDEOPHASER_VIDEO_RENDER_H

/**
 * 在视频解码器 VideoDecoder 中，会在完成解码后调用渲染器中的 Render() 方法。
 * */
#include <stdint.h>
#include <jni.h>
#include "../../utils/one_frame.h"
class VideoRender {
public:
    virtual void InitRender(JNIEnv *env, int video_width, int video_height, int *dst_size) = 0;
    virtual void Render(OneFrame *one_frame) = 0;
    virtual void ReleaseRender() = 0;
};
#endif //VIDEOPHASER_VIDEO_RENDER_H
