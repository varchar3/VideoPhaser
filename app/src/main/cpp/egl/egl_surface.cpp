//
// Created by Administrator on 3/24/2022.
//

#include "egl_surface.h"
#include "../utils/logger.h"

EglSurface::EglSurface() {
    m_core = new EglCore();
}

EglSurface::~EglSurface() {
    delete m_core;
}

bool EglSurface::Init() {
    return m_core->Init(nullptr);
}

void EglSurface::CreateEglSurface(ANativeWindow *native_window, int width, int height) {
    if (native_window != nullptr) {
        this->m_native_window = native_window;
        m_surface = m_core->CreateWindSurface(m_native_window);
    } else {
        m_surface = m_core->CreateOffScreenSurface(width, height);
    }
    if (m_surface == nullptr) {
        LOGE(TAG, "Failed to create surface")
        Release();
    }
    MakeCurrent();
}

void EglSurface::SwapBuffers() {
    m_core->SwapBuffers(m_surface);
}

void EglSurface::MakeCurrent() {
    m_core->MakeCurrent(m_surface);
}

void EglSurface::DestroyEglSurface() {
    if (m_surface != nullptr) {
        if (m_core != nullptr) {
            m_core->DestroySurface(m_surface);
        }
        m_surface = nullptr;
    }
}

void EglSurface::Release() {
    DestroyEglSurface();
    if (m_core != nullptr) {
        m_core->Release();
    }
}