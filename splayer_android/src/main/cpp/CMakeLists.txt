# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# 设置库名称
project(splayer_andorid)

# 设置cmake最低版本
cmake_minimum_required(VERSION 3.4.1)

# 设置GCC编译器的编译选项
if (CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-declarations -Wmissing-prototypes")
endif (CMAKE_COMPILER_IS_GNUCC)

# 设置clang编译器的编译选项
if (CMAKE_COMPILER_IS_CLANG)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-declarations -Wmissing-prototypes -Wdocumentation -Wunreachable-code")
endif (CMAKE_COMPILER_IS_CLANG)

get_filename_component(ROOT_DIR ${CMAKE_SOURCE_DIR}/../../../../ ABSOLUTE)

# 设置依赖库路径
get_filename_component(DISTRIBUTION_ANDROID_DIR ${ROOT_DIR}/distribution/android/ ABSOLUTE)

# 设置ffmpeg依赖库
add_library(splayer_ffmpeg SHARED IMPORTED)
set_target_properties(splayer_ffmpeg PROPERTIES IMPORTED_LOCATION ${DISTRIBUTION_ANDROID_DIR}/ffmpeg/lib/${ANDROID_ABI}/libsffmpeg.so)

# 设置openssl依赖库
add_library(splayer_openssl_crypto STATIC IMPORTED)
set_target_properties(splayer_openssl_crypto PROPERTIES IMPORTED_LOCATION ${DISTRIBUTION_ANDROID_DIR}/openssl/lib/${ANDROID_ABI}/libcrypto.a)

# 设置openssl依赖库
add_library(splayer_openssl_ssl STATIC IMPORTED)
set_target_properties(splayer_openssl_ssl PROPERTIES IMPORTED_LOCATION ${DISTRIBUTION_ANDROID_DIR}/openssl/lib/${ANDROID_ABI}/libssl.a)

#include_directories(
#        # 设置ffmpeg头文件目录
#        ${DISTRIBUTION_ANDROID_DIR}/ffmpeg/include
#        # 设置openssl头文件目录
#        ${DISTRIBUTION_ANDROID_DIR}/openssl/include
#)

message("LOG PROJECT_NAME = ${PROJECT_NAME}")
message("LOG ROOT_DIR = ${ROOT_DIR}")
message("LOG CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")
message("LOG DISTRIBUTION_ANDROID_DIR = ${DISTRIBUTION_ANDROID_DIR}")

# android splayer 实现库
add_subdirectory(${CMAKE_SOURCE_DIR}/impl)

# android splayer 接口库
add_subdirectory(${CMAKE_SOURCE_DIR}/jni)