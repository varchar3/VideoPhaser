//
// Created by Administrator on 3/21/2022.
//

#ifndef VIDEOPHASER_AUDIO_RENDER_H
#define VIDEOPHASER_AUDIO_RENDER_H

#include <cstdint>

/**
 * 由于 NDK 回调JAVA层麻烦，所以直接用 YYDS 的 OpenSL ES 解码 开源，强大
 * 它支持在 native 层直接处理音频数据
 * OpenSL ES 状态机
 * 它的接口不能直接调用，而是要经过对象创建、初始化后，通过对象来调用
 * 通过 Object 的 GetInterface 先获取到接口 Interface ，再通过获取到的 Interface 来调用
 * 1.创建 2.初始化 3.获取
 * DataSource 和 DataSink 数据源 和 输出目标
 * */
class AudioRender{
public:
    virtual void InitRender() = 0;
    virtual void Render(uint8_t *pcm, int size) = 0;
    virtual void ReleaseRender() = 0;
    virtual ~AudioRender() {}
};

#endif //VIDEOPHASER_AUDIO_RENDER_H
