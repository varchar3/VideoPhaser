//
// Created by Administrator on 3/21/2022.
//

#ifndef VIDEOPHASER_OPENSL_RENDER_H
#define VIDEOPHASER_OPENSL_RENDER_H

#include "../audio_render.h"
#include "../../../../utils/logger.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <queue>
#include <thread>

extern "C" {
#include <libavutil/mem.h>
};

class OpenSLRender : public AudioRender {
private:
    //建一个临时对象存取,做为数据缓冲队列的成员
    class PcmData {
    public:
        PcmData(uint8_t *pcm, int size) {
            this->pcm = pcm;
            this->size = size;
        }
        ~PcmData() {//释放操作放在调用时执行
        }
        uint8_t *pcm = nullptr;
        int size = 0;
        bool used = false;
    };

    //创建引擎接口对象
    SLObjectItf m_engine_obj = nullptr;

    SLEngineItf m_engine = nullptr;

    //创建混音器接口对象
    SLObjectItf m_output_mix_obj = nullptr;

    SLEnvironmentalReverbItf m_output_mix_env_reverb = nullptr;
    SLEnvironmentalReverbSettings m_reverb_setting = SL_I3DL2_ENVIRONMENT_PRESET_DEFAULT;

    //创建PCM播放器接口对象
    SLObjectItf m_pcm_player_obj = nullptr;

    SLPlayItf m_pcm_player = nullptr;
    SLVolumeItf m_pcm_player_volume = nullptr;

    //缓冲队列接口
    SLAndroidSimpleBufferQueueItf m_pcm_buffer;

    //入队帧FIFO
    std::queue<PcmData *> m_data_queue;

    pthread_mutex_t m_cache_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t m_cache_cond = PTHREAD_COND_INITIALIZER;

    //创建引擎
    bool CreateEngine();

    //创建混音器
    bool CreateOutputMixer();

    //创建播放器
    bool CreatePlayer();

    //开始播放渲染
    void StartRender();

    //音频数据压入缓冲队列
    void BlockEnqueue();

    //检查是否播放错误
    bool CheckError(SLresult result, std::string hint);

    void static sRenderPcm(OpenSLRender *that);

    /**
     * sReadPcmBufferCbFun 是一个静态方法，可以推测出，OpenSL ES 播放音频内部是一个独立的线程，
     * 这个线程不断的读取缓冲区的数据，进行渲染，并在数据渲染完了以后，通过这个回调接口通知我们填充新数据。
     */
    void static sReadPcmBufferCbFun(SLAndroidSimpleBufferQueueItf bufferQueueItf, void *context);

    void WaitForCache() {
        pthread_mutex_lock(&m_cache_mutex);
        pthread_cond_wait(&m_cache_cond, &m_cache_mutex);
        pthread_mutex_unlock(&m_cache_mutex);
    }

    void SendCacheReadySignal() {
        pthread_mutex_lock(&m_cache_mutex);
        pthread_cond_signal(&m_cache_cond);
        pthread_mutex_unlock(&m_cache_mutex);
    }

public:
    const char *TAG = "OpenSLRender";

    const SLuint32 SL_QUEUE_BUFFER_COUNT = 2;

    OpenSLRender();

    ~OpenSLRender();

    void InitRender() override;

    void Render(uint8_t *pcm, int size) override;

    void ReleaseRender() override;
};


#endif //VIDEOPHASER_OPENSL_RENDER_H
