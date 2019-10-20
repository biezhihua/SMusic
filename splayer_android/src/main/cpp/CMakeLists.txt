# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# 设置库名称
project(splayer_andorid)

# 设置cmake最低版本
cmake_minimum_required(VERSION 3.4.1)

# 设置C++版本
set(CMAKE_CXX_STANDARD 11)

# 设置GCC编译器的编译选项
if (CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-declarations -Wmissing-prototypes")
endif (CMAKE_COMPILER_IS_GNUCC)

# 设置clang编译器的编译选项
if (CMAKE_COMPILER_IS_CLANG)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-declarations -Wmissing-prototypes -Wdocumentation -Wunreachable-code")
endif (CMAKE_COMPILER_IS_CLANG)

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

# 设置依赖库头文件
#include_directories(
#
#        # 设置ffmpeg头文件目录
#        ${ANDROID_DISTRIBUTION_ANDROID_DIR}/ffmpeg/include
#
#        # 设置openssl头文件目录
#        ${ANDROID_DISTRIBUTION_ANDROID_DIR}/openssl/include
#
#        # 引入SoundTouch头文件
#        ${ANDROID_ROOT_DIR}/splayer_soundtouch/include
#        ${ANDROID_ROOT_DIR}/splayer_soundtouch/
#
#        # 引入引擎头文件
#        ${ANDROID_ROOT_DIR}/splayer_engine/include
#
#)

# 打印消息
message("LOG ANDROID ANDROID_ROOT_DIR = ${ANDROID_ROOT_DIR}")
message("LOG ANDROID CMAKE_SYSTEM_NAME = ${CMAKE_SYSTEM_NAME}")
message("LOG ANDROID ANDROID_DISTRIBUTION_ANDROID_DIR = ${ANDROID_DISTRIBUTION_ANDROID_DIR}")

# android splayer 实现库
add_subdirectory(${CMAKE_SOURCE_DIR}/impl)

# android splayer 接口库
add_subdirectory(${CMAKE_SOURCE_DIR}/jni)

#target_include_directories(${PROJECT_NAME} PRIVATE
#
#        # 设置ffmpeg头文件目录
#        ${ANDROID_DISTRIBUTION_ANDROID_DIR}/ffmpeg/include
#
#        # 设置openssl头文件目录
#        ${ANDROID_DISTRIBUTION_ANDROID_DIR}/openssl/include
#
#        # 引入SoundTouch头文件
#        ${ANDROID_ROOT_DIR}/splayer_soundtouch/include
#        ${ANDROID_ROOT_DIR}/splayer_soundtouch/
#
#        # 引入引擎头文件
#        ${ANDROID_ROOT_DIR}/splayer_engine/include
#
#        )