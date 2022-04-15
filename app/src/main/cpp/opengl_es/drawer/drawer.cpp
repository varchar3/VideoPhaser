//
// Created by Administrator on 3/25/2022.
//

#include "drawer.h"
#include "../../utils/logger.h"
#include <malloc.h>

Drawer::Drawer(int width, int height) :
m_origin_width(width), m_origin_height(height){
}

Drawer::~Drawer() {
}

void Drawer::SetSize(int width, int height) {
    this->m_origin_width = width;
    this->m_origin_height = height;
}

bool Drawer::IsReadyToDraw() {
    return m_origin_width>0 && m_origin_height>0;
}

void Drawer::Draw(){
    if (IsReadyToDraw()) {
        CreateTextureId();
        CreateProgram();
        BindTexture();
        PrepareDraw();
        DoDraw();
        DoneDraw();
    }
}

void Drawer::DoDraw() {
    //启用顶点句柄
    glEnableVertexAttribArray(m_vertex_pos_handler);
    //启用纹理句柄
    glEnableVertexAttribArray(m_texture_pos_handler);
    //设置顶点着色器参数
    glVertexAttribPointer(m_vertex_pos_handler, 2, GL_FLOAT, GL_FALSE, 0, m_vertex_coors);
    //设置纹理着色器参数
    glVertexAttribPointer(m_texture_pos_handler, 2, GL_FLOAT, GL_FALSE, 0, m_texture_coors);
    //画三角
    glDrawArrays(GL_TRIANGLE_STRIP,0,4);
}

void Drawer::CreateTextureId() {
    if (m_texture_id == 0){
        glGenTextures(1,&m_texture_id);
        LOGI(TAG, "Create texture id : %d, %x", m_texture_id, glGetError())
    }
}

GLuint Drawer::LoadShader(GLenum type, const GLchar *shader_code) {
    LOGI(TAG, "Load shader:\n %s", shader_code)
    //根据type创建顶点着色器或者片元着色器
    GLuint shader = glCreateShader(type);
    //将资源加入到着色器中，并编译
    glShaderSource(shader,1,&shader_code,nullptr);
    glCompileShader(shader);
    GLint compiled;
    //获取编译状态
    glGetShaderiv(shader,GL_COMPILE_STATUS,&compiled);
    //编译不成功状态下，返回1就是成功
    if(!compiled){
        GLint infoLen = 0;
        /**
         * ++++++++ 纠正动编失败一步，分配一个数组到底是为啥啊 ++++++++
         * */
        char sdLog[1024] = {0};//分配内存时候毫无意义的一步？？？或者是动编失败？

        glGetShaderiv(shader,GL_COMPILE_STATUS,&infoLen);
        if (infoLen > 1){
            //打印信息
            GLchar* infoLog = (GLchar*) malloc(sizeof(GLchar) * infoLen);
            glGetShaderInfoLog(shader, infoLen, nullptr, infoLog);
            free(infoLog);
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

void Drawer::CreateProgram() {
    if (m_program_id == 0){
        //1.创建
        m_program_id = glCreateProgram();
        LOGI(TAG, "create gl program : %d, %x", m_program_id, glGetError())
        //2.附着
        GLuint vertexShader = LoadShader(GL_VERTEX_SHADER,GetVertexShader());
        glAttachShader(m_program_id,vertexShader);

        GLuint fragmentShader = LoadShader(GL_FRAGMENT_SHADER,GetFragmentShader());
        glAttachShader(m_program_id,fragmentShader);
        //3.链接
        glLinkProgram(m_program_id);
        //4.获取 接收器 矩阵 位置 纹理 顶点
        m_vertex_matrix_handler = glGetUniformLocation(m_program_id,"uMatrix");
        m_vertex_pos_handler = glGetAttribLocation(m_program_id,"aPosition");
        m_texture_handler = glGetUniformLocation(m_program_id,"uTexture");
        m_texture_pos_handler = glGetAttribLocation(m_program_id,"aCoordinate");

        InitCstShaderHandler();

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }
    if (m_program_id != 0){
        glUseProgram(m_program_id);
    }
}

void Drawer::ActivateTexture(GLenum type, GLuint texture, GLenum index, int texture_handler) {
    if (texture == -1) texture = m_texture_id;
    if (m_texture_handler == -1) texture_handler = m_texture_handler;
    //激活指定纹理单元 GL_TEXTURE0对应0，GL_TEXTURE1对应1
    glActiveTexture(GL_TEXTURE0 + index);
    //绑定纹理ID到纹理单元
    glBindTexture(type,texture);
    //将活动的纹理单元传递到着色器里边
    glUniform1i(texture_handler,index);
    //配置边缘过滤参数
    glTexParameterf(type,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameterf(type,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(type,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(type,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
}

void Drawer::Release() {
    glDisableVertexAttribArray(m_vertex_pos_handler);
    glDisableVertexAttribArray(m_texture_pos_handler);
    glBindTexture(GL_TEXTURE_2D,0);
    glDeleteTextures(1,&m_texture_id);
    glDeleteProgram(m_program_id);
}

