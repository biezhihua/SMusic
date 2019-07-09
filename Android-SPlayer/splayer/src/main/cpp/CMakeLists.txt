# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

set(DISTRIBUTION ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../distribution)
set(SPLAYER ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../splayer)

add_library(sffmpeg SHARED IMPORTED)
set_target_properties(sffmpeg PROPERTIES IMPORTED_LOCATION ${DISTRIBUTION}/ffmpeg/lib/${ANDROID_ABI}/libsffmpeg.so)

add_library(openssl_crypto STATIC IMPORTED)
set_target_properties(openssl_crypto PROPERTIES IMPORTED_LOCATION ${DISTRIBUTION}/openssl/lib/${ANDROID_ABI}/libcrypto.a)

add_library(openssl_ssl STATIC IMPORTED)
set_target_properties(openssl_ssl PROPERTIES IMPORTED_LOCATION ${DISTRIBUTION}/openssl/lib/${ANDROID_ABI}/libssl.a)

include_directories(splayer/android)
aux_source_directory(splayer/android android)

add_subdirectory(${SPLAYER} core)

add_library(splayer SHARED
        SMainJni.cpp
        ${android}
        )

target_include_directories(splayer PRIVATE
        ${DISTRIBUTION}/ffmpeg/include
        ${DISTRIBUTION}/openssl/include
        ${SPLAYER}/player
        ${SPLAYER}/common
        ${SPLAYER}/soundtouch
        )

target_link_libraries(splayer
        android
        log
        sffmpeg
        splayer_core
        openssl_crypto
        openssl_ssl
        OpenSLES)