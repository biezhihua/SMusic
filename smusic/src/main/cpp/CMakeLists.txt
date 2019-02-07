# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

get_filename_component(SPLAYER_ANDROID ${CMAKE_CURRENT_SOURCE_DIR}/../../../.. ABSOLUTE)

if (${ANDROID_ABI} STREQUAL "armeabi-v7a")
    get_filename_component(SPLAYER_FF_OUTPUT ${SPLAYER_ANDROID}/ffmpeg-output/ffmpeg-armv7a ABSOLUTE)
elseif (${ANDROID_ABI} STREQUAL "x86")
    get_filename_component(SPLAYER_FF_OUTPUT ${SPLAYER_ANDROID}/ffmpeg-output/ffmpeg-x86 ABSOLUTE)
endif ()

get_filename_component(SPLAYER_FF_OUTPUT_INCLUDE ${SPLAYER_FF_OUTPUT}/include/ ABSOLUTE)

get_filename_component(SPLAYER_FF_OUTPUT_SO ${SPLAYER_FF_OUTPUT}/lib/libsffmpeg.so ABSOLUTE)

add_library(sffmpeg
        SHARED
        IMPORTED
        )

set_target_properties(sffmpeg
        PROPERTIES IMPORTED_LOCATION
        ${SPLAYER_FF_OUTPUT_SO}
        )

include_directories(${SPLAYER_FF_OUTPUT_INCLUDE})

add_library(
        smusic
        SHARED

#        test/SubThreadCallJavaContext.h
#        test/smusic_jni.cpp

        SMainJni.cpp

        SMedia.cpp
        SMedia.h

        SFFmpeg.cpp
        SFFmpeg.h

        SJavaMethods.cpp
        SJavaMethods.h

        SLog.h

        SPlayer.cpp
        SPlayer.h

        SQueue.cpp
        SQueue.h
)

target_link_libraries(
        smusic
        log
        sffmpeg)