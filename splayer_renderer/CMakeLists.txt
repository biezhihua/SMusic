# 设置cmake最低版本
cmake_minimum_required(VERSION 3.4.1)

# 设置库名称
project(splayer_renderer)

# 指定C++版本
set(CMAKE_CXX_STANDARD 11)

# 设置根目录
get_filename_component(RENDERER_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ ABSOLUTE)

# 添加源码子目录
file(GLOB RENDERER_CPP_FILES
        src/*.cpp
        *.cpp
        )

# 添加头文件子目录
file(GLOB RENDERER_H_FILES
        include/*.h
        *.h
        )

# 设置源文件集合
set(RENDERER_SOURCE_FILES ${RENDERER_CPP_FILES} ${RENDERER_H_FILES})

# 设置库
add_library(${PROJECT_NAME} SHARED ${RENDERER_SOURCE_FILES})

# 链接头文件
target_include_directories(${PROJECT_NAME} PRIVATE

        # 引入引擎代码头文件
        ${RENDERER_ROOT_DIR}/splayer_engine/include
        ${RENDERER_ROOT_DIR}/splayer_engine

        # 引入自有代码头文件
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

# 链接模块
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")

    # 根据API版本判断使用哪个版本的OpenGLES
    if (${ANDROID_PLATFORM_LEVEL} LESS 12)
        message(FATAL_ERROR "OpenGL 2 is not supported before API level 11 \
                      (currently using ${ANDROID_PLATFORM_LEVEL}).")
        return()
    elseif (${ANDROID_PLATFORM_LEVEL} LESS 18)
        add_definitions("-DDYNAMIC_ES3")
        set(GL3STUB_SRC gl3stub.c)
        set(OPENGL_LIB GLESv2)
    else ()
        set(OPENGL_LIB GLESv3)
    endif (${ANDROID_PLATFORM_LEVEL} LESS 12)

    target_link_libraries(${PROJECT_NAME}
            -llog
            -lEGL
            ${OPENGL_LIB}
            )

elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
endif ()