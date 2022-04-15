//
// Created by Administrator on 3/29/2022.
//
/**
 * 默认的代理器
 * 这里通过一个容器来维护多个绘制器 Drawer
 * */

#include "def_drawer_proxy_impl.h"
#include "../../../utils/logger.h"

void DefDrawerProxyImpl::AddDrawer(Drawer *drawer) {
    m_drawers.push_back(drawer);
}

void DefDrawerProxyImpl::Draw() {
    for (int i = 0; i < m_drawers.size(); ++i) {
        m_drawers[i]->Draw();
    }
}

void DefDrawerProxyImpl::Release() {
    for (int i = 0; i < m_drawers.size(); ++i) {
        m_drawers[i]->Release();
        delete m_drawers[i];
    }
    m_drawers.clear();
}