//
// Created by Administrator on 3/30/2022.
//

#include "gl_player.h"
#include "../opengl_es/drawer/proxy/def_drawer_proxy_impl.h"
#include "render/audio/sound_render/opensl_render.h"

GLPlayer::GLPlayer(JNIEnv *jniEnv, jstring path) {
    m_v_decoder = new VideoDecoder(jniEnv, path);

    //OpenGL渲染
    m_v_drawer = new VideoDrawer;
    m_v_decoder->SetRender(m_v_drawer);

    //创建绘制代理器
    auto *proxyImpl = new DefDrawerProxyImpl();
    // 将video drawer 注入绘制代理中
    proxyImpl->AddDrawer(m_v_drawer);

    m_v_drawer_proxy = proxyImpl;

    //创建opengl绘制器
    m_gl_render = new OpenGLRender(jniEnv,m_v_drawer_proxy);
    LOGI("GLPlayer", "init all success!!!")

    //音频解码器
    m_a_decoder = new AudioDecoder(jniEnv,path, false);
    m_a_render = new OpenSLRender();
    m_a_decoder->SetRender(m_a_render);

}

GLPlayer::~GLPlayer() {
    // 此处不需要 delete 成员指针
    // 在BaseDecoder 和 OpenGLRender 中的线程已经使用智能指针，会自动释放相关指针
}

void GLPlayer::SetSurface(jobject surface) {
    if (surface != nullptr){
        LOGI("GLPlayer", "SetSurface success!!!")
        m_gl_render->SetSurface(surface);
    }
}

void GLPlayer::PlayOrPause() {
    if(!m_v_decoder->IsRunning()){
        m_v_decoder->GoOn();
    }else{
        m_v_decoder->Pause();
    }

    if (!m_a_decoder->IsRunning()){
        m_a_decoder->GoOn();
    }else{
        m_a_decoder->Pause();
    }
}

void GLPlayer::Release() {
    m_gl_render->Stop();
    m_v_decoder->Stop();
    m_a_decoder->Stop();
}