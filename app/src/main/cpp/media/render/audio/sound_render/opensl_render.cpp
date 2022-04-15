//
// Created by Administrator on 3/21/2022.
//
/**
 * 先定义需要用到的引擎、混音器、播放器、以及缓冲队列接口、音量调节接口等。
 * 例如：
 * 1、创建接口对象
 * 2、设置混音器
 * 3、创建播放器（录音器）
 * 4、设置缓冲队列和回调函数
 * 5、设置播放状态
 * 6、启动回调函数
 * */
#include <string>
#include <unistd.h>
#include "opensl_render.h"
#include "../../../../utils/logger.h"

OpenSLRender::OpenSLRender() {

}

OpenSLRender::~OpenSLRender() {

}

void OpenSLRender::InitRender() {
    if (!CreateEngine()) return;
    if (!CreateOutputMixer()) return;
    if (!CreatePlayer()) return;
    LOGI(TAG, "OpenSL ES init success")

    std::thread t(sRenderPcm,this);
    t.detach();
}

/**
 * 启动 OpenSL ES 渲染很简单，只需调用播放器的播放接口，并且往缓冲区压入一帧数据，就可以启动渲染流程。
 * */
void OpenSLRender::sRenderPcm(OpenSLRender *that) {
    that->StartRender();
}

void OpenSLRender::StartRender() {
    while (m_data_queue.empty()) {
        WaitForCache();
    }
    (*m_pcm_player)->SetPlayState(m_pcm_player,SL_PLAYSTATE_PLAYING);
    sReadPcmBufferCbFun(m_pcm_buffer,this);
}

void OpenSLRender::ReleaseRender() {
    //设置停止状态
    if (m_pcm_player) {
        (*m_pcm_player)->SetPlayState(m_pcm_player, SL_PLAYSTATE_STOPPED);
        m_pcm_player = nullptr;
    }

    // 先通知回调接口结束，否则可能导致无法销毁：m_pcm_player_obj
    SendCacheReadySignal();

    //销毁播放器
    if (m_pcm_player_obj) {
        (*m_pcm_player_obj)->Destroy(m_pcm_player_obj);
        m_pcm_player_obj = nullptr;
        m_pcm_buffer = nullptr;
    }
    //销毁混音器
    if (m_output_mix_obj) {
        (*m_output_mix_obj)->Destroy(m_output_mix_obj);
        m_output_mix_obj = nullptr;
    }
    //销毁引擎
    if (m_engine_obj) {
        (*m_engine_obj)->Destroy(m_engine_obj);
        m_engine_obj = nullptr;
        m_engine = nullptr;
    }
    //释放缓存数据
    for (int i = 0; i < m_data_queue.size(); ++i) {
        PcmData *pcm = m_data_queue.front();
        m_data_queue.pop();
        delete pcm;
    }
}

/**
 * 开始渲染播放
 * */
void OpenSLRender::Render(uint8_t *pcm, int size) {
    if (m_pcm_player){
        if (pcm != nullptr && size >= 0) {
            // 只缓存两帧数据，避免占用太多内存，导致内存申请失败，播放出现杂音
            while (m_data_queue.size() >= 2) {
                SendCacheReadySignal();
                //函数是把调用该函数的线程挂起一段时间
                usleep(20000);
            }

            // 将数据复制一份，并压入队列
            uint8_t *data = (uint8_t *)malloc(size);
            memcpy(data,pcm,size);

            PcmData *pcmData = new PcmData(pcm,size);
            m_data_queue.push(pcmData);

            //通知播放线程退出等待恢复播放
            SendCacheReadySignal();
        }else{
            free(pcm);
        }
    }
}

/**
 * 创建引擎
 * */
bool OpenSLRender::CreateEngine() {
    SLresult result = slCreateEngine(
            &m_engine_obj,
            0,
            nullptr,
            0,
            nullptr,
            nullptr);
    if (CheckError(result,"Engine Create Error")) return false;

    result = (*m_engine_obj)->Realize(m_engine_obj,SL_BOOLEAN_FALSE);
    if (CheckError(result,"Engine Realize Error")) return false;

    result = (*m_engine_obj)->GetInterface(m_engine_obj,SL_IID_ENGINE,&m_engine);

    return !CheckError(result, "Engine Interface");
}

/**
 * 设置混音器
 * */
bool OpenSLRender::CreateOutputMixer() {
    const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};
    SLresult result = (*m_engine)->CreateOutputMix(m_engine,&m_output_mix_obj,1, mids, mreq);
    if (CheckError(result,"Create OutputMix Error")) return false;

    result = (*m_output_mix_obj)->Realize(m_output_mix_obj,SL_BOOLEAN_FALSE);
    if (CheckError(result,"Realize OutputMix Error")) return false;

    return true;
}

/**
 * 创建播放器
 * */
bool OpenSLRender::CreatePlayer() {
    //【1.配置数据源 DataSource】----------------------
    //配置PCM格式信息,参数2为队列缓存数,SLDataFormat_PCM直接按要求配就行
    SLDataLocator_AndroidBufferQueue android_queue = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,SL_QUEUE_BUFFER_COUNT};
    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,
            (SLuint32)2,//双声道
            SL_SAMPLINGRATE_44_1,//44100
            SL_PCMSAMPLEFORMAT_FIXED_16,//16位采样位深
            SL_PCMSAMPLEFORMAT_FIXED_16,//同上
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//左右立体声
            SL_BYTEORDER_LITTLEENDIAN//结束标致
    };
    //数据源设置
    SLDataSource slDataSource = {&android_queue,&pcm};

    //【2.配置输出 DataSink】----------------------
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX,m_output_mix_obj};
    SLDataSink slDataSink = {&outputMix,nullptr};

    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE,SL_IID_EFFECTSEND,SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE};

    //【3.创建播放器】----------------------
    SLresult result = (*m_engine)->CreateAudioPlayer(m_engine, &m_pcm_player_obj,
                                                     &slDataSource, &slDataSink, 3, ids, req);
    if (CheckError(result, "Create Player Error")) return false;

    result = (*m_pcm_player_obj)->Realize(m_pcm_player_obj,SL_BOOLEAN_FALSE);
    if (CheckError(result, "Realize Player Error")) return false;

    //【4.获取播放器接口】----------------------
    //得到接口后调用，获取Player接口
    result = (*m_pcm_player_obj)->GetInterface(m_pcm_player_obj,SL_IID_PLAY,&m_pcm_player);
    if (CheckError(result, "Player GetInterface Error")) return false;
    //获取音量接口
    result = (*m_pcm_player_obj)->GetInterface(m_pcm_player_obj,SL_IID_VOLUME,&m_pcm_player_volume);
    if (CheckError(result, "Player GetVolume Error")) return false;

    //【5. 获取缓冲队列接口】----------------------
    //注册回调缓冲区，获取缓冲队列接口
    result = (*m_pcm_player_obj)->GetInterface(m_pcm_player_obj,SL_IID_BUFFERQUEUE,&m_pcm_buffer);
    if (CheckError(result, "Player GetPCMBufferQueue Error")) return false;
    //注册缓冲接口回调
    result = (*m_pcm_buffer)->RegisterCallback(m_pcm_buffer,sReadPcmBufferCbFun,this);
    if (CheckError(result, "Player RegisterCallback Error")) return false;

    LOGI(TAG, "OpenSL ES init success")
    return true;
}

void OpenSLRender::sReadPcmBufferCbFun(SLAndroidSimpleBufferQueueItf bufferQueueItf, void *context) {
    OpenSLRender *player =(OpenSLRender *) context;
    player->BlockEnqueue();
}

/**
 * 对数据进行入队操作
 * 首先，将 m_data_queue 中已经使用的数据先删除，回收资源；
 * 接着，判断是否还有未播放的缓冲数据，没有则进入等待；
 * 最后，通过 (*m_pcm_buffer)->Enqueue() 方法，将数据压入 OpenSL 队列。
 * */
void OpenSLRender::BlockEnqueue() {
    if (m_pcm_player == nullptr) return;

    //先将已用过的数据移除
    while (!m_data_queue.empty()) {
        PcmData *pcm = m_data_queue.front();
        if (pcm->used) {
            m_data_queue.pop();
            pcm->pcm = nullptr;
            pcm->used = false;
            free(pcm);
        }else {
            break;
        }
    }

    // 等待数据缓冲
    while (m_data_queue.empty() && m_pcm_player != nullptr) {// if m_pcm_player is NULL, stop render
        WaitForCache();
    }

    PcmData * pcmData = m_data_queue.front();
    if (pcmData != nullptr && m_pcm_player) {
        SLresult result = (*m_pcm_buffer)->Enqueue(m_pcm_buffer,pcmData->pcm,(SLuint32)pcmData->size);
        if (result == SL_RESULT_SUCCESS) {
            //只做已使用标记，下一帧压入前移除，保证数据可用，否则出现破音
            pcmData->used = true;
        }
    }
}

bool OpenSLRender::CheckError(SLresult result, std::string hint) {
    if (SL_RESULT_SUCCESS != result) {
        LOGE(TAG, "OpenSL ES [%s] init fail", hint.c_str())
        return true;
    }
    return false;
}