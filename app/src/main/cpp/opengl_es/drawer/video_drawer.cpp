//
// Created by Administrator on 3/29/2022.
//

#include "video_drawer.h"

VideoDrawer::VideoDrawer(): Drawer(0, 0) {
}

VideoDrawer::~VideoDrawer() {
}

void VideoDrawer::InitRender(JNIEnv *env, int video_width, int video_height, int *dst_size) {
    SetSize(video_width, video_height);
    dst_size[0] = video_width;
    dst_size[1] = video_height;
}

/**
 * 将解码好的画面数据并保存到 cst_data 中
 * */
void VideoDrawer::Render(OneFrame *one_frame) {
    cst_data = one_frame->data;
}

void VideoDrawer::BindTexture() {
    ActivateTexture();
}

/**
 * 在绘制前，将 cst_data 中的数据通过 glTexImage2D 方法，映射到 OpenGL 的 2D 纹理中。
 * */
void VideoDrawer::PrepareDraw() {
    if (cst_data != nullptr) {
        glTexImage2D(GL_TEXTURE_2D, 0, // level一般为0
                     GL_RGBA, //纹理内部格式
                     origin_width(), origin_height(), // 画面宽高
                     0, // 必须为0
                     GL_RGBA, // 数据格式，必须和上面的纹理格式保持一直
                     GL_UNSIGNED_BYTE, // RGBA每位数据的字节数，这里是BYTE: 1 byte
                     cst_data);// 画面数据
    }
}

const char* VideoDrawer::GetVertexShader() {
    const GLbyte shader[] = "attribute vec4 aPosition;\n"
                            "attribute vec2 aCoordinate;\n"
                            "varying vec2 vCoordinate;\n"
                            "void main() {\n"
                            "  gl_Position = aPosition;\n"
                            "  vCoordinate = aCoordinate;\n"
                            "}";
    return (char *)shader;
}

const char* VideoDrawer::GetFragmentShader() {
    const GLbyte shader[] = "precision mediump float;\n"
                            "uniform sampler2D uTexture;\n"
                            "varying vec2 vCoordinate;\n"
                            "void main() {\n"
                            "  vec4 color = texture2D(uTexture,vCoordinate);\n"
                            "  gl_FragColor = color;\n"
                            "}";
    return (char *)shader;
}

void VideoDrawer::ReleaseRender() {
}

void VideoDrawer::InitCstShaderHandler() {

}

void VideoDrawer::DoneDraw() {
}

