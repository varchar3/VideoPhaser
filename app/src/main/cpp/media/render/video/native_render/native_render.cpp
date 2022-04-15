//
// Created by Administrator on 3/4/2022.
//
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include "../video_render.h"
#include "native_render.h"
#include <jni.h>

extern "C" {
#include <libavutil/mem.h>
}

NativeRender::NativeRender(JNIEnv *env, jobject surface) {
    ///设置全局引用
    m_surface_ref = env->NewGlobalRef(surface);
}

NativeRender::~NativeRender() {
}

void NativeRender::InitRender(JNIEnv *env, int video_width, int video_height, int *dst_size) {
    //初始化窗口,将 Surface 绑定给本地窗口
    m_native_window = ANativeWindow_fromSurface(env, m_surface_ref);

    //绘制区域宽高,获取到 Surface 可显示区域的宽高 返回窗口表面的当前宽高度（以像素为单位）。
    int windowWidth = ANativeWindow_getWidth(m_native_window);
    int windowHeight = ANativeWindow_getHeight(m_native_window);

    //视频宽高
    m_dst_w = windowWidth;
    m_dst_h = m_dst_w * video_height / video_width;
    //如果目标高度大于显示屏高度
    if (m_dst_h > windowHeight) {
        m_dst_h = windowHeight;
        m_dst_w = m_dst_h * video_width / video_height;
    }

    //设置宽高限制缓冲区中的像素数量
    ANativeWindow_setBuffersGeometry(
            m_native_window,windowWidth,windowHeight,WINDOW_FORMAT_RGBA_8888);

    dst_size[0] = m_dst_w;
    dst_size[1] = m_dst_h;
}

/**
 * 两个重要的本地方法：
 * ANativeWindow_lock 锁定窗口，并获取到输出缓冲区 m_out_buffer。
 * ANativeWindow_unlockAndPost 释放窗口，并将缓冲数据绘制到屏幕上。
*/
void NativeRender::Render(OneFrame *one_frame) {
    //锁定窗口
    ANativeWindow_lock(m_native_window,&m_out_buffer,nullptr);
    uint8_t *dst = (uint8_t *) m_out_buffer.bits;
    //获取stride：一行可以保存的内存像素数量*4（即：rgba的位数）
    int dst_stride = m_out_buffer.stride * 4;
    int src_stride = one_frame->line_size;

    // 由于window的stride和帧的stride不同，因此需要逐行复制
    for (int k = 0;k<m_dst_h; k++) {
        memcpy(dst + k * dst_stride, one_frame->data + k * src_stride,src_stride);
    }
    //释放窗口
    ANativeWindow_unlockAndPost(m_native_window);
}

void NativeRender::ReleaseRender() {
    if (m_native_window != nullptr) {
        ANativeWindow_release(m_native_window);
    }
    av_free(&m_out_buffer);
}