//
// Created by Administrator on 3/24/2022.
//

#include <thread>
#include <android/native_window_jni.h>
#include <unistd.h>
#include "opengl_render.h"
#include "../utils/logger.h"

OpenGLRender::OpenGLRender(JNIEnv *env, DrawerProxy *drawer_proxy) :m_drawer_proxy(drawer_proxy) {
    this->env = env;
    //获取jvm虚拟机
    env->GetJavaVM(&m_jvm_for_thread);
    InitRenderThread();
}

OpenGLRender::~OpenGLRender() {
    delete m_egl_surface;
}

void OpenGLRender::InitRenderThread() {
    //使用智能指针，线程结束时自动删除
    std::shared_ptr<OpenGLRender> that(this);
    std::thread t(sRenderThread, that);
    t.detach();
}

void OpenGLRender::sRenderThread(std::shared_ptr<OpenGLRender> that) {
    JNIEnv *env;

    //将线程附加到虚拟机，并获取env
    if (that->m_jvm_for_thread->AttachCurrentThread(&env, nullptr) != JNI_OK) {
        LOGI(that->TAG, "thread init faild")
        return;
    }

    if (!that->InitEGL()){
        //解除线程和jvm关联
        that->m_jvm_for_thread->DetachCurrentThread();
        return;
    }

    while (true) {
        switch (that->m_state) {
            case FRESH_SURFACE:
                LOGI(that->TAG, "Loop Render FRESH_SURFACE")
                that->InitDspWindow(env);
                that->CreateSurface();
                that->m_state = RENDERING;
                break;
            case RENDERING:
                LOGI(that->TAG, "Loop Render RENDERING")
                that->Render();
                break;
            case SURFACE_DESTROY:
                LOGI(that->TAG, "Loop Render SURFACE_DESTROY")
                that->DestroySurface();
                that->m_state = NO_SURFACE;
                break;
            case STOP:
                LOGI(that->TAG, "Loop Render STOP,检测到播放退出")
                //解除线程和jvm关联
                that->ReleaseRender();
                that->m_jvm_for_thread->DetachCurrentThread();
                return;
            case NO_SURFACE:
            default:
                break;
        }
        usleep(20000);
    }
}

void OpenGLRender::DestroySurface() {
    m_egl_surface->DestroyEglSurface();
    ReleaseWindow();
}

bool OpenGLRender::InitEGL() {
    m_egl_surface = new EglSurface();
    return m_egl_surface->Init();
}

void OpenGLRender::SetSurface(jobject surface) {
    if (nullptr != surface) {
        m_surface_ref = env->NewGlobalRef(surface);
        m_state = FRESH_SURFACE;
    } else {
        env->DeleteGlobalRef(m_surface_ref);
        m_state = SURFACE_DESTROY;
    }
}

void OpenGLRender::InitDspWindow(JNIEnv *env) {
    if (m_surface_ref != nullptr) {
        m_native_window = ANativeWindow_fromSurface(env, m_surface_ref);

        m_window_width = ANativeWindow_getWidth(m_native_window);
        m_window_height = ANativeWindow_getHeight(m_native_window);

        ANativeWindow_setBuffersGeometry(m_native_window, m_window_width, m_window_height,
                                         WINDOW_FORMAT_RGBA_8888);
    }
}

void OpenGLRender::CreateSurface() {
    m_egl_surface->CreateEglSurface(m_native_window, m_window_width, m_window_height);
    glViewport(0, 0, m_window_width, m_window_height);
}

void OpenGLRender::Render() {
    if (RENDERING == m_state) {
        m_drawer_proxy->Draw();
        //交换缓冲数据显示
        m_egl_surface->SwapBuffers();
    }
}

void OpenGLRender::Stop() {
    m_state = STOP;
}

void OpenGLRender::ReleaseRender() {
    ReleaseDrawers();
    ReleaseSurface();
    ReleaseWindow();
}

void OpenGLRender::ReleaseDrawers() {
    if (m_drawer_proxy != nullptr) {
        m_drawer_proxy->Release();
        delete m_drawer_proxy;
        m_drawer_proxy = nullptr;
    }
}

void OpenGLRender::ReleaseSurface() {
    if (m_egl_surface != nullptr) {
        m_egl_surface->Release();
        delete m_egl_surface;
        m_egl_surface = nullptr;
    }
}

void OpenGLRender::ReleaseWindow() {
    if (m_native_window != nullptr) {
        ANativeWindow_release(m_native_window);
        m_native_window = nullptr;
    }
}