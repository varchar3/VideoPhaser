//
// Created by Administrator on 2/25/2022_
//

/**
 * 将播放器暴露给 Java 层使用了  JNI EXPORT !!!
 * */
#include "native_lib.h"
#include <string>
#include <unistd.h>
#include <jni.h>
#include "media/player.h"
#include "media/gl_player.h"

extern "C" {
#include <libavcodec/jni.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>

/**
 * return the configuration of FFmpeg
 * */
JNIEXPORT jstring JNICALL
Java_com_nmgb_vp_activity_SecondActivity_stringFormJNI(JNIEnv *env, jobject/* this */) {
    return env->NewStringUTF(avcodec_configuration());
}

/**
 * 创建播放器，并返回播放器对象地址
 * */
JNIEXPORT jint JNICALL
Java_com_nmgb_vp_activity_SecondActivity_createPlayer(
        JNIEnv *env,
        jobject/* this */,
        jstring path,
        jobject surface) {
    auto *player = new Player(env, path, surface);

    return (jint) player;
}

JNIEXPORT jint JNICALL
Java_com_nmgb_vp_activity_SecondActivity_play(JNIEnv *env, jobject/* this */, jint player) {
    auto *p = (Player *) player;
    p->play();
    return 1;
}

JNIEXPORT jint JNICALL
Java_com_nmgb_vp_activity_SecondActivity_pause(JNIEnv *env, jobject/* this */, jint player) {
    auto *p = (Player *) player;
    p->pause();
    return 0;
}
/**--------------------------------OPENGL+EGL 做代理渲染 --------------------------------**/
JNIEXPORT jint JNICALL
Java_com_nmgb_vp_activity_FFmpegGLPlayerActivity_createGLPlayer(
        JNIEnv *env,
        jobject/* this */,
        jstring path,
        jobject surface) {
    auto *glPlayer = new GLPlayer(env, path);
    glPlayer->SetSurface(surface);
    return (jint) glPlayer;
}

JNIEXPORT void JNICALL
Java_com_nmgb_vp_activity_FFmpegGLPlayerActivity_glPlayOrPause(
        JNIEnv *env, jobject/* this */, jint glPlayer) {
    auto *p = (GLPlayer *) glPlayer;
    p->PlayOrPause();
}

JNIEXPORT void JNICALL
Java_com_nmgb_vp_activity_FFmpegGLPlayerActivity_glStop(
        JNIEnv *env, jobject/* this */, jint glPlayer) {
    auto *p = (GLPlayer *) glPlayer;
    p->Release();
}

}