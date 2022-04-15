//
// Created by Administrator on 3/29/2022.
//

#ifndef VIDEOPHASER_DEF_DRAWER_PROXY_IMPL_H
#define VIDEOPHASER_DEF_DRAWER_PROXY_IMPL_H

#include "drawer_proxy.h"

class DefDrawerProxyImpl : public DrawerProxy {
private:
    std::vector<Drawer *> m_drawers;

public:
    void AddDrawer(Drawer *drawer);
    void Draw() override;
    void Release() override;
};

#endif //VIDEOPHASER_DEF_DRAWER_PROXY_IMPL_H
