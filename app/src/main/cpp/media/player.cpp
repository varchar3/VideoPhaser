//
// Created by Administrator on 3/8/2022.
//

#include "player.h"
#include "render/video/native_render/native_render.h"
#include "render/audio/sound_render/opensl_render.h"
#include "../opengl_es/drawer/proxy/def_drawer_proxy_impl.h"

Player::Player(JNIEnv *env, jstring path, jobject surface) {
    //先初始化视频解码器
    m_v_decoder = new VideoDecoder(env, path,false);
    //再初始化视频渲染器
    m_v_render = new NativeRender(env, surface);

    //初始化音频解码器
    m_a_decoder = new AudioDecoder(env,path,false);
    //初始化音频渲染器
    m_a_render = new OpenSLRender();

    //再设置一下渲染器
    m_v_decoder->SetRender(m_v_render);
    m_a_decoder->SetRender(m_a_render);
}

Player::~Player() {
    // 此处不需要 delete 成员指针
    // 在BaseDecoder中的线程已经使用智能指针，会自动释放
}

void Player::play() {
    if (m_v_decoder != nullptr) {
        m_v_decoder->GoOn();
        m_a_decoder->GoOn();
    }
}

void Player::pause() {
    if (m_v_decoder != nullptr) {
        m_v_decoder->Pause();
        m_a_decoder->GoOn();
    }
}