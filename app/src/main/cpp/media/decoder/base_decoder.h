//
// Created by Administrator on 2/25/2022.
//

#ifndef VIDEOPHASER_BASE_DECODER_H
#define VIDEOPHASER_BASE_DECODER_H

#include <jni.h>
#include <string.h>
#include <thread>
#include "i_decoder.h"
#include "decode_state.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
#include <libavutil/time.h>
}

class BaseDecoder : public IDecoder {

public:
    //-------------constructor and 构析函数->释放内存-------------------------
    BaseDecoder(JNIEnv *env, jstring path, bool b);

    virtual ~BaseDecoder();

    const char *TAG = "Base_Decoder";

    //--------实现基础类方法-----------------

    void GoOn() override;

    void Pause() override;

    void Stop() override;

    bool IsRunning() override;

    long GetDuration() override;

    long GetCurPos() override;

    int width() {
        return m_codec_ctx->width;
    }

    int height() {
        return m_codec_ctx->height;
    }

    AVPixelFormat video_pixel_format() {
        return m_codec_ctx->pix_fmt;
    }

    AVCodecContext* get_codec_context() {
        return m_codec_ctx;
    }

private:

    //-------------define about the decoding!------------------------------
    //解码器上下文
    AVCodecContext *m_codec_ctx = NULL;

    //解码信息上下文
    AVFormatContext *m_format_ctx = NULL;

    //解码器
    const AVCodec *m_codec = NULL;

    //待解码包
    AVPacket *m_packet = NULL;

    //最后解码出来的帧 一般用于存储原始数据 即非压缩数据 例如对视频来说是YUV，RGB，对音频来说是PCM
    AVFrame *m_frame = NULL;

    //当前播放时间
    int64_t m_cur_t_s = 0;

    //开始播放的时间
    int64_t m_started_t = -1;

    //总时长
    long m_duration = 0;

    //解码状态
    DecodeState m_state = STOP;

    //数据流索引 track
    int m_stream_index = -1;

    // 为合成器提供解码
    bool m_for_synthesizer = false;

    //-----------------私有方法------------------------------
    /**
     * 初始化FFMpeg相关的参数
     * @param env jvm环境
     */
    void InitFFMpegDecoder(JNIEnv *env);

    /**
     * 分配解码过程中需要的缓存
     * */
    void AllocateFrameBuffer();

    /**
     * 循环解码
     * */
    void LoopDecoding();

    /**
     * 获取当前时间戳
     * */
    void ObtainTimeStamp();

    /**
     * 解码完成
     * */
    void DoneDecode(JNIEnv *env);

    /**
     * 时间同步
     * */
    void SyncRender();

    // -------------------定义线程相关-----------------------------
    // 线程依附的JVM环境
    JavaVM *m_jvm_for_thread = NULL;

    // 原始路径jstring引用，否则无法在线程中操作
    jobject m_path_ref = NULL;

    // 经过转换的路径
    const char *m_path = NULL;

    //线程等待解锁变量
    pthread_mutex_t m_mutex = PTHREAD_MUTEX_INITIALIZER;//初始化互斥锁
    pthread_cond_t m_cond = PTHREAD_COND_INITIALIZER;//创建互斥量cond

//    IDecoderStateCb *m_state_cb  = NULL;

    /**
     * 新建解码器
     * */
    void CreateDecodeThread();

    /**
     * 静态解码方法，用于解码线程回调
     * @param that 当前解码器
     */
    static void Decode(std::shared_ptr<BaseDecoder> that);

    void CallbackState(DecodeState status);

protected:
    bool ForSynthesizer() {
        return m_for_synthesizer;
    }

    /**
     * 音视频索引,0 1
     */
    virtual AVMediaType GetMediaType() = 0;

    /**
     * 解码的数据帧 一帧
     * */
    AVFrame *DecodeOneFrame();

    /**
     * 进入等待
     * */
    void Wait(long second = 0,long ms = 0);

    /**
     * 恢复状态 激活等待列表中的线程
     * */
    void SendSignal();

    /**
     * 子类准备回调方法
     * @note 注：在解码线程中回调
     */
    virtual void Prepare(JNIEnv *env) = 0;

    /**
     * 子类渲染回调方法
     * @note 注：在解码线程中回调
     * @param frame 视频：一帧YUV数据；音频：一帧PCM数据
     */
    virtual void Render(AVFrame *frame) = 0;

    /**
     * 子类释放资源回调方法
     */
    virtual void Release() = 0;

    void Init(JNIEnv *env, jstring path);
};


#endif //VIDEOPHASER_BASE_DECODER_H
