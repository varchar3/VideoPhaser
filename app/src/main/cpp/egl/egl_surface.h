//
// Created by Administrator on 3/24/2022.
//

#ifndef VIDEOPHASER_EGL_SURFACE_H
#define VIDEOPHASER_EGL_SURFACE_H


#include <android/native_window.h>
#include "egl_core.h"

class EglSurface {
private:
    const char *TAG = "EglSurface";

    ANativeWindow *m_native_window = nullptr;

    EglCore *m_core;

    EGLSurface m_surface;

public:
    EglSurface();
    ~EglSurface();

    bool Init();
    void CreateEglSurface(ANativeWindow *native_window,int width,int height);
    void MakeCurrent();
    void SwapBuffers();
    void DestroyEglSurface();
    void Release();
};


#endif //VIDEOPHASER_EGL_SURFACE_H
