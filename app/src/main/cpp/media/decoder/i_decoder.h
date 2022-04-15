//
// Created by Administrator on 2/25/2022.
//

#ifndef VIDEOPHASER_I_DECODER_H
#define VIDEOPHASER_I_DECODER_H
class IDecoder{
public:
    virtual void GoOn() = 0;
    virtual void Pause() = 0;
    virtual void Stop() = 0;
    virtual bool IsRunning() = 0;
    virtual long GetDuration() = 0;
    virtual long GetCurPos() = 0;
};

#endif //VIDEOPHASER_I_DECODER_H
