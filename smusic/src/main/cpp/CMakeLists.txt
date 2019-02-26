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

include_directories(soundtouch/include)
include_directories(soundtouch/SoundTouch)

add_library(
        smusic
        SHARED

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

        SError.h

        SStatus.h

        SOpenSLES.cpp
        SOpenSLES.h

        soundtouch/SoundTouch/AAFilter.cpp
        soundtouch/SoundTouch/BPMDetect.cpp
        soundtouch/SoundTouch/cpu_detect_x86.cpp
        soundtouch/SoundTouch/FIFOSampleBuffer.cpp
        soundtouch/SoundTouch/FIRFilter.cpp
        soundtouch/SoundTouch/InterpolateCubic.cpp
        soundtouch/SoundTouch/InterpolateLinear.cpp
        soundtouch/SoundTouch/InterpolateShannon.cpp
        soundtouch/SoundTouch/mmx_optimized.cpp
        soundtouch/SoundTouch/PeakFinder.cpp
        soundtouch/SoundTouch/RateTransposer.cpp
        soundtouch/SoundTouch/SoundTouch.cpp
        soundtouch/SoundTouch/sse_optimized.cpp
        soundtouch/SoundTouch/TDStretch.cpp

)

target_link_libraries(
        smusic
        log
        sffmpeg
        OpenSLES)