//
// Created by Administrator on 3/30/2022.
//

#ifndef VIDEOPHASER_GL_PLAYER_H
#define VIDEOPHASER_GL_PLAYER_H

#include "../opengl_es/opengl_render.h"
#include "../opengl_es/drawer/proxy/drawer_proxy.h"
#include "../opengl_es/drawer/video_drawer.h"
#include "decoder/video/v_decoder.h"
#include "decoder/audio/a_decoder.h"
class GLPlayer {
private:
    VideoDecoder *m_v_decoder;
    OpenGLRender *m_gl_render;

    DrawerProxy *m_v_drawer_proxy;
    VideoDrawer *m_v_drawer;

    AudioDecoder *m_a_decoder;
    AudioRender *m_a_render;

public:
    GLPlayer(JNIEnv *jniEnv, jstring path);
    ~GLPlayer();

    void SetSurface(jobject surface);
    void PlayOrPause();
    void Release();
};


#endif //VIDEOPHASER_GL_PLAYER_H
