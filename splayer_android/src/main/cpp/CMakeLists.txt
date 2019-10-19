# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.
project(splayer_andorid)

cmake_minimum_required(VERSION 3.4.1)

## 设置依赖库路径
#set(DISTRIBUTION_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../distribution/android/)
#
## 设置播放器引擎目录
#set(SPLAYER_ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../splayer_engine)
#
## 设置SoundTouch目录
#set(SPLAYER_SOUND_TOUCH_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../splayer_soundtouch)
#
## 添加引擎子模块目录
#add_subdirectory(${SPLAYER_ENGINE_DIR} alias_splayer_engine)
#
## 引入FFmpegLib
#add_library(splayer_ffmpeg SHARED IMPORTED)
#set_target_properties(splayer_ffmpeg PROPERTIES IMPORTED_LOCATION ${DISTRIBUTION_DIR}/ffmpeg/lib/${ANDROID_ABI}/libsffmpeg.so)
#
## 引入OpenSSLLib
#add_library(splayer_openssl_crypto STATIC IMPORTED)
#set_target_properties(splayer_openssl_crypto PROPERTIES IMPORTED_LOCATION ${DISTRIBUTION_DIR}/openssl/lib/${ANDROID_ABI}/libcrypto.a)
#add_library(splayer_openssl_ssl STATIC IMPORTED)
#set_target_properties(splayer_openssl_ssl PROPERTIES IMPORTED_LOCATION ${DISTRIBUTION_DIR}/openssl/lib/${ANDROID_ABI}/libssl.a)
#
## 设置代码源
#aux_source_directory(src src_source)
#
## 组合Lib
#add_library(${PROJECT_NAME} SHARED
#        SMainJni.cpp
#        ${src_source}
#        )
#
#target_include_directories(${PROJECT_NAME} PRIVATE
#
#        # 引入FFmpeg头文件
#        ${DISTRIBUTION_DIR}/ffmpeg/include
#
#        # 引入OPENSSL头文件
#        ${DISTRIBUTION_DIR}/openssl/include
#
#        # 引入SoundTouch头文件
#        ${SPLAYER_SOUND_TOUCH_DIR}/include
#        ${SPLAYER_SOUND_TOUCH_DIR}
#
#        # 引入引擎头文件
#        ${SPLAYER_ENGINE_DIR}/include
#
#        # 引入自有代码头文件
#        ${CMAKE_CURRENT_SOURCE_DIR}/include
#
#        )
#
#target_link_libraries(${PROJECT_NAME}
#        log
#        splayer_engine
#        splayer_ffmpeg
#        splayer_openssl_ssl
#        splayer_openssl_crypto
#        OpenSLES)