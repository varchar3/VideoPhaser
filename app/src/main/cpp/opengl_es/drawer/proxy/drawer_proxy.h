//
// Created by Administrator on 3/25/2022.
//
/**
 * 绘制代理--只作为接口使用
 * 把 Drawer 的调用交给它来实现
 * 只有绘制和释放两个外部方法
 * */
#ifndef VIDEOPHASER_DRAWER_PROXY_H
#define VIDEOPHASER_DRAWER_PROXY_H

#include <vector>
#include "../drawer.h"

class DrawerProxy {
private:
    std::vector<Drawer *> m_drawers;
public:
    virtual void Draw() = 0;
    virtual void Release() = 0;
    virtual ~DrawerProxy() {}
};


#endif //VIDEOPHASER_DRAWER_PROXY_H
