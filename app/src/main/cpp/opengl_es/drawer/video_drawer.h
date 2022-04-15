//
// Created by Administrator on 3/29/2022.
//

#ifndef VIDEOPHASER_VIDEO_DRAWER_H
#define VIDEOPHASER_VIDEO_DRAWER_H
/**
 * 继承视频渲染器
 * 虽然我们已经定义了 OpenGLRender 来渲染 OpenGL，但是并没有继承自 VideoRender ，
 * 同时前面说过，OpenGLRender 会调用代理渲染器来实现真正的绘制。
 * */

#include "drawer.h"
#include "../../media/render/video/video_render.h"

class VideoDrawer : public Drawer, public VideoRender {
public:
    VideoDrawer();

    ~VideoDrawer();

    // 实现 VideoRender 定义的方法
    void InitRender(JNIEnv *env, int video_width, int video_height, int *dst_size) override;

    void Render(OneFrame *one_frame) override;

    void ReleaseRender() override;

    // 实现子类定义的方法
    const char *GetVertexShader() override;

    const char *GetFragmentShader() override;

    void InitCstShaderHandler() override;

    void BindTexture() override;

    void PrepareDraw() override;

    void DoneDraw() override;

};


#endif //VIDEOPHASER_VIDEO_DRAWER_H
