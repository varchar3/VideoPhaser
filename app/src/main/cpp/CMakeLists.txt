# cmake 最低版本
cmake_minimum_required(VERSION 3.4.1)

# 支持gnu++11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

# 1. 定义so库和头文件所在目录，方面后面使用
set(ffmpeg_lib_dir ${CMAKE_SOURCE_DIR}/../../../libs/${ANDROID_ABI})
set(ffmpeg_head_dir ${CMAKE_SOURCE_DIR}/ffmpeg)

# 2. 添加头文件目录
include_directories(${ffmpeg_head_dir}/include)

# 3. 添加ffmpeg相关的so库
# add_library第一参数是so名字，SHARED 表示引入方式为动态库引入。
add_library(
        avutil
        SHARED
        IMPORTED
)

set_target_properties(
        avutil
        PROPERTIES IMPORTED_LOCATION
        ${ffmpeg_lib_dir}/libavutil.so
)

add_library(
        swresample
        SHARED
        IMPORTED
)
set_target_properties(
        swresample
        PROPERTIES IMPORTED_LOCATION
        ${ffmpeg_lib_dir}/libswresample.so
)

add_library(
        swscale
        SHARED
        IMPORTED
)
set_target_properties(
        swscale
        PROPERTIES IMPORTED_LOCATION
        ${ffmpeg_lib_dir}/libswscale.so
)

add_library(
        avcodec
        SHARED
        IMPORTED
)
set_target_properties(
        avcodec
        PROPERTIES IMPORTED_LOCATION
        ${ffmpeg_lib_dir}/libavcodec.so
)

add_library(
        avdevice
        SHARED
        IMPORTED
)
set_target_properties(
        avdevice
        PROPERTIES IMPORTED_LOCATION
        ${ffmpeg_lib_dir}/libavdevice.so
)

add_library(
        avfilter
        SHARED
        IMPORTED
)
set_target_properties(
        avfilter
        PROPERTIES IMPORTED_LOCATION
        ${ffmpeg_lib_dir}/libavfilter.so
)

add_library(
        avformat
        SHARED
        IMPORTED
)
set_target_properties(
        avformat
        PROPERTIES IMPORTED_LOCATION
        ${ffmpeg_lib_dir}/libavformat.so
)

# 查找代码中使用到的系统库
find_library(
        # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# 配置so库编译信息
add_library(
        # 输出so库的名称
        native-lib

        # 设置生成库的方式，默认为SHARE动态库
        SHARED

        # 列出参与编译的所有源文件
        ${CMAKE_SOURCE_DIR}/utils/timer.c
        ${CMAKE_SOURCE_DIR}/utils/const.h
        # 播放器
        ${CMAKE_SOURCE_DIR}/media/player.h
        ${CMAKE_SOURCE_DIR}/media/player.cpp

        ${CMAKE_SOURCE_DIR}/media/gl_player.h
        ${CMAKE_SOURCE_DIR}/media/gl_player.cpp

        # 解码器
        ${CMAKE_SOURCE_DIR}/utils/one_frame.h

        ${CMAKE_SOURCE_DIR}/media/decoder/i_decoder.h
        ${CMAKE_SOURCE_DIR}/media/decoder/decode_state.h
        ${CMAKE_SOURCE_DIR}/media/decoder/base_decoder.h
#        ${CMAKE_SOURCE_DIR}/media/decoder/i_decode_state_cb.h
        ${CMAKE_SOURCE_DIR}/media/render/audio/sound_render/opensl_render.h
        ${CMAKE_SOURCE_DIR}/media/render/audio/sound_render/opensl_render.cpp
        ${CMAKE_SOURCE_DIR}/media/decoder/base_decoder.cpp
        ${CMAKE_SOURCE_DIR}/media/decoder/video/v_decoder.cpp
        ${CMAKE_SOURCE_DIR}/media/decoder/audio/a_decoder.cpp

        #opengl相关
        ${CMAKE_SOURCE_DIR}/egl/egl_core.cpp
        ${CMAKE_SOURCE_DIR}/egl/egl_surface.cpp
        ${CMAKE_SOURCE_DIR}/opengl_es/opengl_render.cpp
        ${CMAKE_SOURCE_DIR}/opengl_es/drawer/drawer.cpp
        ${CMAKE_SOURCE_DIR}/opengl_es/drawer/drawer.h
        ${CMAKE_SOURCE_DIR}/opengl_es/drawer/proxy/def_drawer_proxy_impl.cpp
        ${CMAKE_SOURCE_DIR}/opengl_es/drawer/video_drawer.cpp
        ${CMAKE_SOURCE_DIR}/opengl_es/drawer/proxy/drawer_proxy.cpp

        # 渲染器
        ${CMAKE_SOURCE_DIR}/media/render/video/video_render.h
        ${CMAKE_SOURCE_DIR}/media/render/video/native_render/native_render.cpp
        native_lib.cpp)


# 指定编译目标库时，cmake要链接的库
target_link_libraries(
        # 指定目标库，native-lib 是在上面 add_library 中配置的目标库
        native-lib
        # 列出所有需要链接的库
        ${log-lib}

        # 4. 连接 FFmpeg 相关的库
        avutil
        avcodec
        avdevice
        avformat
        avfilter
        swscale
        swresample
        android

        # 打开opensl es支持
        OpenSLES
        # 打开opengl es支持
        EGL
        GLESv2

)