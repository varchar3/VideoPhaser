//
// Created by Administrator on 3/8/2022.
//

#ifndef VIDEOPHASER_PLAYER_H
#define VIDEOPHASER_PLAYER_H
#include "decoder/video/v_decoder.h"
#include "decoder/audio/a_decoder.h"

class Player{
private:
    VideoDecoder *m_v_decoder;
    VideoRender *m_v_render;

    AudioDecoder *m_a_decoder;
    AudioRender *m_a_render;
public:
    Player(JNIEnv *jniEnv, jstring path, jobject surface);
    ~Player();

    void play();
    void pause();
};
#endif //VIDEOPHASER_PLAYER_H
