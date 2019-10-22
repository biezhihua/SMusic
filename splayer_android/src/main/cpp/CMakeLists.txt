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
add_subdirectory(${CMAKE_SOURCE_DIR}/impl)

# android splayer 接口库
add_subdirectory(${CMAKE_SOURCE_DIR}/jni)

