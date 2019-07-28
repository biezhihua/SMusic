# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.
project(splayer)

cmake_minimum_required(VERSION 3.4.1)

set(DISTRIBUTION_DIR /Users/biezhihua/StudySpace/SPlayer/distribution)
set(SPLAYER_DIR /Users/biezhihua/StudySpace/SPlayer/splayer)

add_library(splayer_ffmpeg SHARED IMPORTED)
set_target_properties(splayer_ffmpeg PROPERTIES IMPORTED_LOCATION ${DISTRIBUTION_DIR}/ffmpeg/lib/${ANDROID_ABI}/libsffmpeg.so)

add_library(splayer_openssl_crypto STATIC IMPORTED)
set_target_properties(splayer_openssl_crypto PROPERTIES IMPORTED_LOCATION ${DISTRIBUTION_DIR}/openssl/lib/${ANDROID_ABI}/libcrypto.a)

add_library(splayer_openssl_ssl STATIC IMPORTED)
set_target_properties(splayer_openssl_ssl PROPERTIES IMPORTED_LOCATION ${DISTRIBUTION_DIR}/openssl/lib/${ANDROID_ABI}/libssl.a)

add_library(splayer_core STATIC IMPORTED)
set_target_properties(splayer_core PROPERTIES IMPORTED_LOCATION ${SPLAYER_DIR}/libsplayer_core.a)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src splayer_android)

add_library(${PROJECT_NAME} SHARED
        SMainJni.cpp
        ${splayer_android}
        )

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${DISTRIBUTION_DIR}/ffmpeg/include
        ${DISTRIBUTION_DIR}/openssl/include
        ${SPLAYER_DIR}/include
        )

target_link_libraries(${PROJECT_NAME}
        log
        splayer_core
        splayer_ffmpeg
        splayer_openssl_ssl
        splayer_openssl_crypto
        OpenSLES)