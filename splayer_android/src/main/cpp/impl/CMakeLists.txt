# 设置库名称
project(splayer_andorid_impl)

# 设置cmake最低版本
cmake_minimum_required(VERSION 3.4.1)

# 根据API版本判断使用哪个版本的OpenGLES
if (${ANDROID_PLATFORM_LEVEL} LESS 18)
    add_definitions("-DDYNAMIC_ES3")
    set(GLES-lib GLESv2)
else ()
    set(GLES-lib GLESv3)
endif (${ANDROID_PLATFORM_LEVEL} LESS 11)

# 添加引擎子模块目录
add_subdirectory(${ANDROID_ROOT_DIR}/splayer_engine splayer_engine)

# 添加源码子目录
file(GLOB ANDROID_IMPL_CPP_FILES src/*.cpp)

# 添加头文件子目录
file(GLOB ANDROID_IMPL_H_FILES include/*.h)

# 设置源文件集合
set(ANDROID_IMPL_SOURCE_FILES ${ANDROID_IMPL_CPP_FILES} ${ANDROID_IMPL_H_FILES})

# 设置头文件
include_directories(

        # 设置ffmpeg头文件目录
        ${ANDROID_DISTRIBUTION_ANDROID_DIR}/ffmpeg/include

        # 设置openssl头文件目录
        ${ANDROID_DISTRIBUTION_ANDROID_DIR}/openssl/include

        # 引入SoundTouch头文件
        ${ANDROID_ROOT_DIR}/splayer_soundtouch/include
        ${ANDROID_ROOT_DIR}/splayer_soundtouch/

        # 引入引擎头文件
        ${ANDROID_ROOT_DIR}/splayer_engine/include

        # 引入自有代码头文件
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 设置库
add_library(${PROJECT_NAME} ${ANDROID_IMPL_SOURCE_FILES})


# 链接静态库
target_link_libraries(${PROJECT_NAME}
        splayer_engine
        -landroid
        -llog
        -lOpenSLES
        -lEGL
        ${GLES-lib})
