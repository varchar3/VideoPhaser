//
// Created by cxp on 2019-08-05.
//
/**
 * 1  秒＝1000毫秒，ms
 * 1毫秒＝1000微秒，μs
 * 1微妙＝1000纳秒，ns
 * 1纳秒＝1000皮秒。ps
 */
#include "sys/time.h"
#include "logger.h"

/**
 * tv_sec以秒为单位的时间戳
 * tv_usec以微秒为单位的时间戳
 * */
int64_t GetCurMsTime() {
    struct timeval tv;
    //gettimeofday函数得到更精确的时间
    gettimeofday(&tv, NULL);
    int64_t ts = (int64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
    LOGI("DoneDecode","%ld---%ld",tv.tv_sec,tv.tv_usec)
    return ts;
}