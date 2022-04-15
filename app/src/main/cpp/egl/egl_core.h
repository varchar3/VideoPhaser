//
// Created by Administrator on 3/24/2022.
//

#ifndef VIDEOPHASER_EGL_CORE_H
#define VIDEOPHASER_EGL_CORE_H

extern "C" {
#include <EGL/egl.h>
#include <EGL/eglext.h>
};

class EglCore {
private:
    const char *TAG = "EglCore";

    //显示窗口
    EGLDisplay m_egl_dsp = EGL_NO_DISPLAY;
    //EGL上下文
    EGLContext m_egl_ctx = EGL_NO_CONTEXT;
    //EGL配置
    EGLConfig m_egl_cfg;

    EGLConfig GetEGLConfig();

public:
    EglCore();

    ~EglCore();

    bool Init(EGLContext share_ctx);

    //根据本地窗口创建显示界面
    EGLSurface CreateWindSurface(ANativeWindow *window);

    EGLSurface CreateOffScreenSurface(int width, int height);

    //将opengl上下文和线程进行绑定
    void MakeCurrent(EGLSurface egl_surface);

    //将缓存数据交到前台显示
    void SwapBuffers(EGLSurface egl_surface);

    //释放显示
    void DestroySurface(EGLSurface egl_surface);

    //释放EGL
    void Release();

};


#endif //VIDEOPHASER_EGL_CORE_H
