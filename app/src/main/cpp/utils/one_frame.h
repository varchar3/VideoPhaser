/**
 * 一个数据（音频/视频）帧数据
 * */

#include <malloc.h>
extern "C" {
#include <libavutil/rational.h>
}

class OneFrame{
public:
    uint8_t *data = NULL;
    int line_size;
    uint64_t pts;
    AVRational time_base;
    uint8_t *ext_data = NULL;

    // 是否自动回收data和ext_data
    bool autoRecycle = true;

    OneFrame(uint8_t *data, int line_size, int64_t pts, AVRational time_base,
             uint8_t *ext_data = NULL, bool autoRecycle = true) {//构造
        this->data = data;
        this->line_size = line_size;
        this->pts = pts;
        this->time_base = time_base;
        this->autoRecycle = autoRecycle;
        this->ext_data = ext_data;
    }

    ~OneFrame(){//释放
        if (autoRecycle){
            if (data!=NULL){
                free(data);
                data = NULL;
            }
            if (ext_data != NULL) {
                free(ext_data);
                ext_data = NULL;
            }
        }
    }
};


