# 设置库名称
project(splayer_andorid_jni)

# 设置cmake最低版本
cmake_minimum_required(VERSION 3.10.2)

# 添加源码子目录
file(GLOB ANDROID_JNI_CPP_FILES src/*.cpp)

# 添加头文件子目录
file(GLOB ANDROID_JNI_H_FILES include/*.h)

# 设置源文件集合
set(ANDROID_JNI_SOURCE_FILES ${ANDROID_JNI_CPP_FILES} ${ANDROID_JNI_H_FILES})

# 设置头文件
include_directories(

        # 设置ffmpeg头文件目录
        ${ANDROID_DISTRIBUTION_ANDROID_DIR}/ffmpeg/include

        # 设置openssl头文件目录
        ${ANDROID_DISTRIBUTION_ANDROID_DIR}/openssl/include

        # 引入SoundTouch头文件
        ${ANDROID_ROOT_DIR}/splayer_soundtouch/include
        ${ANDROID_ROOT_DIR}/splayer_soundtouch

        # 引入引擎头文件
        ${ANDROID_ROOT_DIR}/splayer_engine/include
        ${ANDROID_ROOT_DIR}/splayer_engine

        # 引入自有代码头文件
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}

        # 设置IMPL头文件目录
        ${CMAKE_SOURCE_DIR}/impl/include
)

# 设置库
add_library(${PROJECT_NAME}
        SHARED
        ${ANDROID_JNI_SOURCE_FILES})

target_link_libraries(${PROJECT_NAME}
        splayer_andorid_impl
        splayer_ffmpeg
        -landroid
        -llog
        )
