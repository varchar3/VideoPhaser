//
// Created by Administrator on 3/25/2022.
//
/**
 * 绘制器
 * */

#ifndef VIDEOPHASER_DRAWER_H
#define VIDEOPHASER_DRAWER_H

extern "C" {
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
}

class Drawer {
private:
    const char *TAG = "Drawer";
    /**上下颠倒的顶点矩阵*/
    const GLfloat m_reserve_vertex_coors[8] = {
            -1.0f, 1.0f,
            1.0f, 1.0f,
            -1.0f, -1.0f,
            1.0f, -1.0f
    };

    const GLfloat m_vertex_coors[8] = {
            -1.0f, -1.0f,
            1.0f, -1.0f,
            -1.0f, 1.0f,
            1.0f, 1.0f
    };

    const GLfloat m_texture_coors[8] = {
            0.0f, 1.0f,
            1.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f
    };

    GLuint m_program_id = 0;

    GLuint m_texture_id = 0;

    GLint m_vertex_matrix_handler = -1;

    GLint m_vertex_pos_handler = -1;

    GLint m_texture_pos_handler = -1;

    GLint m_texture_handler = -1;

    int m_origin_width = 0;

    int m_origin_height = 0;

    void CreateTextureId();

    void CreateProgram();

    GLuint LoadShader(GLenum type, const GLchar *shader_code);

    void DoDraw();

public:
    Drawer(int width, int height);
    virtual ~Drawer();

    void Draw();

    bool IsReadyToDraw();

    void Release();

    int origin_width() {
        return m_origin_width;
    }

    int origin_height() {
        return m_origin_height;
    }

protected:
    // 自定义用户数据，可用于存放画面数据
    void *cst_data = nullptr;

    void SetSize(int width, int height);

    void ActivateTexture(GLenum type = GL_TEXTURE_2D, GLuint texture = -1,
                         GLenum index = 0, int texture_handler = -1);

    // 纯虚函数，子类实现
    virtual const char *GetVertexShader() = 0;

    virtual const char *GetFragmentShader() = 0;

    virtual void InitCstShaderHandler() = 0;

    virtual void BindTexture() = 0;

    virtual void PrepareDraw() = 0;

    virtual void DoneDraw() = 0;
};


#endif //VIDEOPHASER_DRAWER_H
