# 设置库名称
project(splayer_andorid)

# 设置cmake最低版本
cmake_minimum_required(VERSION 3.10.2)

# 设置C++版本
set(CMAKE_CXX_STANDARD 11)

# 设置Root目录
get_filename_component(ANDROID_ROOT_DIR ${CMAKE_SOURCE_DIR}/../../../../ ABSOLUTE)

# 设置依赖库路径
get_filename_component(ANDROID_DISTRIBUTION_ANDROID_DIR ${ANDROID_ROOT_DIR}/distribution/android/ ABSOLUTE)

# 设置ffmpeg依赖库
add_library(splayer_ffmpeg SHARED IMPORTED)
set_target_properties(splayer_ffmpeg PROPERTIES IMPORTED_LOCATION ${ANDROID_DISTRIBUTION_ANDROID_DIR}/ffmpeg/lib/${ANDROID_ABI}/libsffmpeg.so)

# 设置openssl依赖库
add_library(splayer_openssl_crypto STATIC IMPORTED)
set_target_properties(splayer_openssl_crypto PROPERTIES IMPORTED_LOCATION ${ANDROID_DISTRIBUTION_ANDROID_DIR}/openssl/lib/${ANDROID_ABI}/libcrypto.a)

# 设置openssl依赖库
add_library(splayer_openssl_ssl STATIC IMPORTED)
set_target_properties(splayer_openssl_ssl PROPERTIES IMPORTED_LOCATION ${ANDROID_DISTRIBUTION_ANDROID_DIR}/openssl/lib/${ANDROID_ABI}/libssl.a)

# android splayer 实现库
#add_subdirectory(${CMAKE_SOURCE_DIR}/impl)

# android splayer 接口库
#add_subdirectory(${CMAKE_SOURCE_DIR}/jni)


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
file(GLOB ANDROID_CPP_FILES
        src/*.cpp
        )

# 添加头文件子目录
file(GLOB ANDROID_H_FILES
        include/*.h
        )

# 设置源文件集合
set(ANDROID_SOURCE_FILES ${ANDROID_CPP_FILES} ${ANDROID_H_FILES})

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
)

# 设置库
add_library(${PROJECT_NAME}
        SHARED
        ${ANDROID_SOURCE_FILES})

# EGL
# https://woshijpf.github.io/android/2017/09/04/Android%E7%B3%BB%E7%BB%9F%E5%9B%BE%E5%BD%A2%E6%A0%88OpenGLES%E5%92%8CEGL%E4%BB%8B%E7%BB%8D.html
# 链接静态库
target_link_libraries(${PROJECT_NAME}
        splayer_engine
        -landroid
        -llog
        -lEGL
        -lOpenSLES
        ${GLES-lib}
        )
