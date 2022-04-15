//
// Created by Administrator on 2/25/2022.
//
/**
 * 这个结构体主要做：（大部分工作）
 * 1.初始化 m_format_ctx，打开流文件并提取所有信息，包括分轨等
 * 2.初始化 m_codec_ctx，解码器初始化，由 m_format_ctx获取注册codec分配参数
 * 3.分配 m_packet （待解码数据）和 m_frame（解码后数据）
 * 4.Prepare（env）交给了子类 v_decoder.cpp处理，主要是初始化渲染器和适配宽高大小等作用
 * 5.进入解码循环:这边需要做的事情是循环解码线程，Wait主要是等待解码流失的时间做同步处理，
 *   SendSignal给线程重新开启运行
 *   DecodeOneFrame解码一帧 packet里边的数据
 *     av_read_frame 从 m_format_ctx 中读取一帧解封好的待解码数据，存放在 m_packet 中
 *     avcodec_send_packet 将 m_packet 发送到解码器中解码，解码好的数据存放在 m_codec_ctx 中
 *     avcodec_receive_frame接收一帧解码好的数据，存放在 m_frame 中，返回的是（解码后）的 m_frame
 *     释放 packet
 *   ObtainTimeStamp获取当前时间戳
 * */
#include "base_decoder.h"
#include "../../utils/timer.c"
#include "../../utils/logger.h"

BaseDecoder::BaseDecoder(JNIEnv *env, jstring path, bool b) {
    Init(env, path);
    CreateDecodeThread();
}

BaseDecoder::~BaseDecoder() {
    delete m_format_ctx;
    delete m_codec_ctx;
    delete m_frame;
    delete m_packet;
}

void BaseDecoder::Init(JNIEnv *env, jstring path) {
    m_path_ref = env->NewGlobalRef(path);
    m_path = env->GetStringUTFChars(path, nullptr);
    //获取JVM虚拟机，为创建线程作准备
    env->GetJavaVM(&m_jvm_for_thread);
}

void BaseDecoder::CreateDecodeThread() {
    // 使用智能指针，线程结束时，自动删除本类指针
    std::shared_ptr<BaseDecoder> that(this);
    std::thread t(Decode, that);
    t.detach();
}

void BaseDecoder::Decode(std::shared_ptr<BaseDecoder> that) {
    JNIEnv *env;
    //将线程附加到虚拟机，并获取env
    if (that->m_jvm_for_thread->AttachCurrentThread(&env, nullptr) != JNI_OK) {
        LOGE("InitFFMpegDecoder","Fail to Init decode thread \n")
        return;
    }
    //初始化解码器
    that->InitFFMpegDecoder(env);
    //分配解码帧数据内存,解码包所需内存，解码后的包所需内存
    that->AllocateFrameBuffer();
    //回调子类方法，通知子类解码器初始化完毕
    that->Prepare(env);
    //进入解码循环
    that->LoopDecoding();
    //退出解码
    that->DoneDecode(env);
    //接触线程和jvm关联,AttachCurrentThread过后一定要DetachCurrentThread
    that->m_jvm_for_thread->DetachCurrentThread();
}

void BaseDecoder::InitFFMpegDecoder(JNIEnv *env) {
    //1.初始化/分配格式上下文对象
    m_format_ctx = avformat_alloc_context();
    //2.打开文件
    //该函数的作用就是打开文件，尽可能的收集各方面的信息并填充AVFormatContext结构体，
    // 基本上是做了除过解码之外的所有工作。0 on success,
    if (avformat_open_input(&m_format_ctx, m_path, nullptr, nullptr) != 0) {
        DoneDecode(env);
        return;
    }
    //3.获取音频视频流信息  >= 0 if OK,
    if (avformat_find_stream_info(m_format_ctx, nullptr) < 0) {
        DoneDecode(env);
        return;
    }
    /**
     * 4.查找编解码器
     * nb_streams获取该MPEG 有几个轨道，音轨 视轨一般两个
     * */
    //4.1 获取视频流的节点
    int vIdx = -1;
    for (int i = 0; i < m_format_ctx->nb_streams; ++i) {
        if (m_format_ctx->streams[i]->codecpar->codec_type == GetMediaType()) {
            vIdx = i;
            break;
        }
    }

    if (vIdx == -1) {
        LOGE("InitFFMpegDecoder","UNKNOWN VIDEO TYPE\n" )
        DoneDecode(env);
        return;
    }

    m_stream_index = vIdx;

    //4.2获取解码器参数,从这一步最好是获取到avcodec_alloc_context3所需要的参数 m_codec
    AVCodecParameters *codecPar = m_format_ctx->streams[vIdx]->codecpar;

    //4.3获取解码器
    m_codec = avcodec_find_decoder(codecPar->codec_id);

    //4.4获取解码器上下文 创建
    m_codec_ctx = avcodec_alloc_context3(m_codec);
    if (avcodec_parameters_to_context(m_codec_ctx, codecPar) != 0) {//初始化具体内容
        LOGE("InitFFMpegDecoder","Fail to find stream index  \n" )
        DoneDecode(env);
        return;
    }

    //5.打开解码器 在使用此函数之前，必须使用 avcodec_alloc_context3() 分配上下文 the same:zero on success,
    if (avcodec_open2(m_codec_ctx, m_codec, nullptr) < 0) {
        LOGE("InitFFMpegDecoder","Fail to open av codec \n" )
        DoneDecode(env);
        return;
    }

    //总时长AV_TIME_BASE = 1 * 10^6 us
    m_duration = (long) ((float) m_format_ctx->duration / AV_TIME_BASE * 1000);
    LOGI("InitFFMpegDecoder","Decoder init success duration = %ld\n" ,m_duration)
}

void BaseDecoder::AllocateFrameBuffer() {
    //初始化解码和待解码数据,分配内存而已
    //both filled with default values or nullptr on failure.
    m_packet = av_packet_alloc();//待解码的数据
    m_frame = av_frame_alloc();//解码后的数据
}

/**
 * 解码循环
 * */
void BaseDecoder::LoopDecoding() {
    if (STOP == m_state) {
        m_state = START;
    }
    LOGI("LoopDecoding","Start loop decode \n")
    while (true) {
        if (m_state != DECODING &&
            m_state != START &&
            m_state != STOP) {
            //把线程停一下
            Wait();
            // ---------【同步时间矫正】-------------
            //恢复同步的起始时间，即去除等待流失的时间
            //GetCurMsTime()获取系统当前时间戳-当前播放时间时间戳
            m_started_t = GetCurMsTime() - m_cur_t_s;
        }

        if (m_state == STOP) {
            break;
        }

        if (-1 == m_started_t) {
            m_started_t = GetCurMsTime();
        }

        if (DecodeOneFrame() != nullptr) {
            //同步下渲染器，在画面上渲染出来并同步一下
            SyncRender();
            Render(m_frame);

            if (START == m_state) {
                m_state = PAUSE;
            }
        } else {
            LOGI("state","m_state = %d \n", m_state)
            if (ForSynthesizer()) {
                m_state = STOP;
            } else {
                m_state = FINISH;
            }
        }
    }
}

/**
 * av_read_frame(m_format_ctx, m_packet)：
 * 从 m_format_ctx 中读取一帧解封好的待解码数据，存放在 m_packet 中；
 * avcodec_send_packet(m_codec_ctx, m_packet)：
 * 将 m_packet 发送到解码器中解码，解码好的数据存放在 m_codec_ctx 中；
 * avcodec_receive_frame(m_codec_ctx, m_frame)：
 * 接收一帧解码好的数据，存放在 m_frame 中。
 */
AVFrame *BaseDecoder::DecodeOneFrame() {
    int ret = av_read_frame(m_format_ctx, m_packet);
    while (ret == 0) {
        if (m_packet->stream_index == m_stream_index) {
            switch (avcodec_send_packet(m_codec_ctx, m_packet)) {
                case AVERROR_EOF: {
                    av_packet_unref(m_packet);
                    LOGE("DecodeOneFrame","Decode error: %s! \n", av_err2str(AVERROR_EOF))
                    return nullptr;//解码结束
                }
                case AVERROR(EAGAIN): {
                    LOGE("DecodeOneFrame","Decode error: %s! \n", av_err2str(EAGAIN))
                    break;
                }
                case AVERROR(EINVAL): {
                    LOGE("DecodeOneFrame","Decode error: %s! \n", av_err2str(EINVAL))
                    break;
                }
                case AVERROR(ENOMEM): {
                    LOGE("DecodeOneFrame","Decode error: %s! \n", av_err2str(ENOMEM))
                    break;
                }
                default:
                    break;
            }
            /**
             * the main functional to receive the decoded data -> avcodec_receive_frame
             * 不为 0 则是其它错误详见该方法头文件描述
             * 调用一次avcodec_send_packet之后，可能需要调用25次 ,
             * avcodec_receive_frame才能获取全部的解码音频数据
             * */
            int result = avcodec_receive_frame(m_codec_ctx, m_frame);
            if (result == 0) {
                ObtainTimeStamp();
                //擦拭数据包
                av_packet_unref(m_packet);
                return m_frame;
            } else {
                av_log(nullptr, AV_LOG_ERROR, "Receive frame error result! \n");
            }
        }
        // 释放packet
        av_packet_unref(m_packet);
        ret = av_read_frame(m_format_ctx, m_packet);
    }
    av_packet_unref(m_packet);
    LOGI("result","ret = %d! \n", ret)
    return nullptr;
}

/**
 * 获取当前时间戳 单位：毫秒 25FPS换算 ffmpeg 25*60*60
 * DTS（Decoding Time Stamp）：表示压缩帧的解码时间戳 0.040-0.080-0.120-0.160
 * PTS（Presentation Time Stamp）：表示将压缩帧解码后得到的原始帧的显示时间戳 0.080-0.120-0.160-0.200
 * av_q2d 负责把AVRational结构转换成double，通过这个可以计算出某一帧在视频中的时间位置
 * time_base 是一个分数 例如：time_base{1,900000} 这样的形式存在不同的封装格式 time_base也不一样，
 * 不同的封装格式具有不同的时间基。在 FFmpeg 处理音视频过程中的不同阶段，也会采用不同的时间基。
 * ffmpeg的时间基线等同于AV_TIME_BASE_Q
 * */
void BaseDecoder::ObtainTimeStamp() {
    if (m_frame->pkt_dts != AV_NOPTS_VALUE) {//若有解码时间戳
        m_cur_t_s = m_packet->dts;
    }else if (m_frame->pts != AV_NOPTS_VALUE) {//若有显示时间戳
        m_cur_t_s = m_frame->pts;
    }else {
        m_cur_t_s = 0;
    }
    m_cur_t_s = (int64_t)(m_cur_t_s * av_q2d(m_format_ctx->streams[m_stream_index]->time_base)*1000);
//    LOGI("m_cur_t_s","avformat_find_stream_info failed: %lld", destNSec)
}

void BaseDecoder::SyncRender() {
    if (ForSynthesizer()) {
        return;
    }
    int64_t ct = GetCurMsTime();
    int64_t passTime = ct - m_started_t;

    if (m_cur_t_s > passTime) {
        av_usleep( (unsigned int)((m_cur_t_s - passTime)*1000));
    }
}

/**
 * 进入等待线程
 * mutex会帮助我们锁定一段逻辑区域的访问。
 * attention:(but,if 一个数据对象有多处调用的情况,我们需要根据实际情况,设计统一的接口)
 * 加锁->等待->解锁
 * */
void BaseDecoder::Wait(long second, long ms) {
    //互斥锁锁定当前线程，其它线程访问它则休眠，直至它解锁后给到其它线程
    pthread_mutex_lock(&m_mutex);
    if (second > 0 || ms > 0) {
        //s-ms,秒到毫秒 now--现在的时间
        timeval now;
        //s-ns,秒到微秒 outTime--等待已经过去的时间
        timespec outTime;
        gettimeofday(&now, nullptr);
        //长整形声明,这边linux为long long::现在的毫秒+需要等待的毫秒ms * 1000000
        int64_t destNSec = now.tv_usec * 1000 + ms * 1000000;
        outTime.tv_sec = static_cast<__kernel_time_t>(now.tv_sec + second + destNSec / 1000000000);
        //微妙取余 10^9
        outTime.tv_nsec = static_cast<long>(destNSec % 1000000000);
        //线程等待一定的时间，如果超时或有信号触发，线程唤醒。
        pthread_cond_timedwait(&m_cond, &m_mutex, &outTime);
    } else {
        //此时等待线程等待信号触发，如果没有信号触发，无限期等待下去。
        pthread_cond_wait(&m_cond, &m_mutex);
    }
    pthread_mutex_unlock(&m_mutex);
}

/**
 * 恢复全部等待的线程
 * 发送信号量时，也要有三步：加锁->发送->解锁
 * */
void BaseDecoder::SendSignal() {
    pthread_mutex_lock(&m_mutex);
    pthread_cond_signal(&m_cond);
    pthread_mutex_unlock(&m_mutex);
}

void BaseDecoder::GoOn(){
    m_state = DECODING;
    SendSignal();
}

void BaseDecoder::Pause() {
    m_state = PAUSE;
}

void BaseDecoder::Stop() {
    m_state = STOP;
    SendSignal();
}

bool BaseDecoder::IsRunning() {
    return DECODING == m_state;
}

long BaseDecoder::GetDuration() {
    return m_duration;
}

long BaseDecoder::GetCurPos() {
    return (long)m_cur_t_s;
}

//void BaseDecoder::CallbackState(DecodeState status) {
//    if (m_state_cb != NULL) {
//        switch (status) {
//            case PREPARE:
//                m_state_cb->DecodePrepare(this);
//                break;
//            case START:
//                m_state_cb->DecodeReady(this);
//                break;
//            case DECODING:
//                m_state_cb->DecodeRunning(this);
//                break;
//            case PAUSE:
//                m_state_cb->DecodePause(this);
//                break;
//            case FINISH:
//                m_state_cb->DecodeFinish(this);
//                break;
//            case STOP:
//                m_state_cb->DecodeStop(this);
//                break;
//        }
//    }
//}

//解码完毕，释放资源
void BaseDecoder::DoneDecode(JNIEnv *env) {
    LOGI("DoneDecode","Decode decode & release")
    //释放缓存
    if (m_packet != nullptr) {
        av_packet_free(&m_packet);
    }
    if (m_frame != nullptr) {
        av_frame_free(&m_frame);
    }
    // 关闭解码器
    if (m_codec_ctx != nullptr) {
        avcodec_close(m_codec_ctx);
        avcodec_free_context(&m_codec_ctx);
    }
    // 关闭输入流
    if (m_format_ctx != nullptr) {
        avformat_close_input(&m_format_ctx);
        avformat_free_context(m_format_ctx);
    }
    //释放转换参数
    if (m_path_ref != nullptr && m_path != nullptr) {
        env->ReleaseStringUTFChars((jstring)m_path_ref, nullptr);
        env->DeleteGlobalRef(m_path_ref);
    }
    //通知子类释放资源
    Release();
}