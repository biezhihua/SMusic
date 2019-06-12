# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

set(SPLAYER_ANDROID ${CMAKE_CURRENT_SOURCE_DIR}/../../../../distribution)

add_library(sffmpeg SHARED IMPORTED)
set_target_properties(sffmpeg PROPERTIES IMPORTED_LOCATION ${SPLAYER_ANDROID}/ffmpeg/lib/${ANDROID_ABI}/libsffmpeg.so)

add_library(openssl_crypto STATIC IMPORTED)
set_target_properties(openssl_crypto PROPERTIES IMPORTED_LOCATION ${SPLAYER_ANDROID}/openssl/lib/${ANDROID_ABI}/libcrypto.a)

add_library(openssl_ssl STATIC IMPORTED)
set_target_properties(openssl_ssl PROPERTIES IMPORTED_LOCATION ${SPLAYER_ANDROID}/openssl/lib/${ANDROID_ABI}/libssl.a)

# SoundTouch
include_directories(soundtouch/include soundtouch/src)
aux_source_directory(soundtouch/src SoundTouch)

# SPlayer
include_directories(splayer/include)
aux_source_directory(splayer/src SPlayer)

include_directories(splayer/mutex)
aux_source_directory(splayer/mutex Mutex)

add_library(splayer SHARED
        SMainJni.cpp
        ${SPlayer}
        ${SoundTouch}
        ${Mutex}
        )

target_include_directories(splayer PRIVATE
        ${SPLAYER_ANDROID}/ffmpeg/include
        ${SPLAYER_ANDROID}/openssl/include
        )

target_link_libraries(splayer
        android
        log
        sffmpeg
        openssl_crypto
        openssl_ssl
        OpenSLES)