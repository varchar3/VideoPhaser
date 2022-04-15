//
// Created by Administrator on 3/24/2022.
//
/**
 * EGL (Embedded Graphics Library/Interface) 具体的窗口系统 for OpenGL ES
 * EGL 可以既可以创建前台渲染表面，也可以创建离屏渲染表面，离屏渲染主要用于后面合成视频的时候使用。
 * */

/**
 * 总结:
 * 1.eglGetDisplay
 * 2.eglInitialize
 * 3.eglCreateContext
 * 4.eglGetConfigAttrib
 * 5.eglChooseConfig
 * 6.eglCreateWindowSurface
 * 7.eglCreatePbufferSurface
 * eglMakeCurrent
 *
 * Display 是一个连接，用于连接设备上的底层窗口系统。所以在调用EGL 方法之前，需要先创建、初始化这个Display连接
 * Context 不是什么神秘的东西，它仅仅是一个容器
 * Surface 则是设计来存储渲染相关的输出数据
 * */
#include "egl_core.h"
#include "../utils/logger.h"

EglCore::EglCore(){}
EglCore::~EglCore(){}

bool EglCore::Init(EGLContext share_ctx) {
    //EGLDisplay有的话就等于初始化过了，返回true吧
    if (m_egl_dsp != EGL_NO_DISPLAY) {
        LOGE(TAG, "EGL already setup")
        return true;
    }

    //EGLContext为空了时候直接设置空吧
    if (share_ctx == nullptr) {
        share_ctx = EGL_NO_CONTEXT;
    }

    //1.再者获取EGL_DEFAULT_DISPLAY
    m_egl_dsp = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    if (m_egl_dsp == EGL_NO_DISPLAY || eglGetError() != EGL_SUCCESS) {
        LOGE(TAG, "EGL init display failed")
        return false;
    }
    //2.m_egl_dsp初始化
    EGLint major_ver, minor_ver;
    EGLBoolean success = eglInitialize(m_egl_dsp, &major_ver, &minor_ver);
    if (success != EGL_TRUE || eglGetError() != EGL_SUCCESS) {
        LOGE(TAG, "EGL INIT FAIL")
        return false;
    }
    LOGI(TAG, "EGL version: %d.%d", major_ver, minor_ver)

    //3.EGL获取配置
    m_egl_cfg = GetEGLConfig();

    const EGLint attr[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
    m_egl_ctx = eglCreateContext(m_egl_dsp, m_egl_cfg, share_ctx, attr);
    if (m_egl_ctx == EGL_NO_CONTEXT) {
        LOGE(TAG, "EGL Create FAIL,error is %x", eglGetError())
        return false;
    }
    //4.获取配置常量
    EGLint egl_format;
    success = eglGetConfigAttrib(m_egl_dsp, m_egl_cfg, EGL_NATIVE_VISUAL_ID, &egl_format);
    if (success != EGL_TRUE || eglGetError() != EGL_SUCCESS) {
        LOGE(TAG, "EGL get config fail")
        return false;
    }

    LOGI(TAG, "EGL init success")
    return true;
}

/**
 * egl配置一览
 * */
EGLConfig EglCore::GetEGLConfig() {
    EGLint numConfigs;
    EGLConfig config;

    static const EGLint CONFIG_ATTRIBS[] = {
            EGL_BUFFER_SIZE, EGL_DONT_CARE,
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BUFFER_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_DEPTH_SIZE, 16,//采样深度s16
            EGL_STENCIL_SIZE, EGL_DONT_CARE,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_NONE//THE END结束标志
    };

    EGLBoolean success = eglChooseConfig(m_egl_dsp, CONFIG_ATTRIBS, &config, 1, &numConfigs);
    if (!success || eglGetError() != EGL_SUCCESS) {
        LOGI(TAG, "EGL config fail")
        return nullptr;
    }
    return config;
}

/**
 * 创建前台渲染表面
 * */
EGLSurface EglCore::CreateWindSurface(ANativeWindow *window) {
    EGLSurface surface = eglCreateWindowSurface(m_egl_dsp, m_egl_cfg, window, 0);
    if (eglGetError() != EGL_SUCCESS) {
        LOGI(TAG, "EGL create window surface fail")
        return nullptr;
    }
    return surface;
}

/**
 * 创建离屏渲染表面
 * */
EGLSurface EglCore::CreateOffScreenSurface(int width, int height) {
    int CONFIG_ATTRIBS[] = {
            EGL_WIDTH, width,
            EGL_HEIGHT, height,
            EGL_NONE
    };
    EGLSurface surface = eglCreatePbufferSurface(m_egl_dsp, m_egl_cfg, CONFIG_ATTRIBS);
    if (eglGetError() != EGL_SUCCESS) {
        LOGI(TAG, "EGL create off screen surface fail")
        return nullptr;
    }
    return surface;
}

void EglCore::MakeCurrent(EGLSurface egl_surface) {
    if (!eglMakeCurrent(m_egl_dsp, egl_surface, egl_surface, m_egl_ctx)) {
        LOGE(TAG, "egl make current failed")
    }
}

void EglCore::SwapBuffers(EGLSurface egl_surface) {
    eglSwapBuffers(m_egl_dsp, egl_surface);
}

void EglCore::DestroySurface(EGLSurface egl_surface) {
    eglMakeCurrent(m_egl_dsp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(m_egl_dsp, egl_surface);
}

void EglCore::Release() {
    if (m_egl_dsp != EGL_NO_DISPLAY) {
        eglMakeCurrent(m_egl_dsp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(m_egl_dsp, m_egl_ctx);
        eglReleaseThread();
        eglTerminate(m_egl_dsp);
    }
    m_egl_dsp = EGL_NO_DISPLAY;
    m_egl_ctx = EGL_NO_CONTEXT;
    m_egl_cfg = nullptr;
}



