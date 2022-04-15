//
// Created by Administrator on 3/24/2022.
//

#ifndef VIDEOPHASER_OPENGL_RENDER_H
#define VIDEOPHASER_OPENGL_RENDER_H


#include <jni.h>
#include <android/native_window.h>
#include <memory>
#include "../egl/egl_surface.h"
#include "drawer/proxy/drawer_proxy.h"

class OpenGLRender {
private:
    const char *TAG = "OpenGLRender";

    //啊哈哈哈哈哈，状态机来喽
    enum STATE {
        NO_SURFACE,//无有效的surface
        FRESH_SURFACE,//有一个已经初始化的新surface
        RENDERING,//初始化完毕，可以开始渲染
        SURFACE_DESTROY,//surface销毁
        STOP//停止绘制
    };

    JNIEnv *env = nullptr;

    // 线程依附的JVM环境
    JavaVM *m_jvm_for_thread = nullptr;

    // Surface引用，必须使用引用，否则无法在线程中操作
    jobject m_surface_ref = nullptr;

    //本地屏幕
    ANativeWindow *m_native_window =nullptr;

    //Egl显示表面
    EglSurface *m_egl_surface = nullptr;

    //代理绘制
    DrawerProxy *m_drawer_proxy = nullptr;

    int m_window_width = 0;
    int m_window_height = 0;

    STATE m_state = NO_SURFACE;

    // 初始化相关的方法
    void InitRenderThread();
    bool InitEGL();
    void InitDspWindow(JNIEnv *env);

    //创建/销毁Surface
    void CreateSurface();
    void DestroySurface();

    //渲染方法
    void Render();

    //释放资源相关方法
    void ReleaseRender();
    void ReleaseDrawers();
    void ReleaseSurface();
    void ReleaseWindow();

    //渲染线程回调
    static void sRenderThread(std::shared_ptr<OpenGLRender> that);

public:
    OpenGLRender(JNIEnv *env,DrawerProxy * drawer_proxy);
    ~OpenGLRender();

    void SetSurface(jobject surface);
    void SetOffScreeSize(int width, int height);
    void Stop();
};


#endif //VIDEOPHASER_OPENGL_RENDER_H
