cmake_minimum_required(VERSION 3.4.1)

project(splayer_core)

set(CMAKE_CXX_STANDARD 14)

set(DISTRIBUTION ${CMAKE_CURRENT_SOURCE_DIR}/../distribution)

include_directories(include)

aux_source_directory(src src)
aux_source_directory(extra/soundtouch extra_soundtouch)

add_library(splayer_core
        ${include}
        ${src}
        ${extra_soundtouch}
        )

target_include_directories(splayer_core PRIVATE
        ${DISTRIBUTION}/ffmpeg/include
        src
        extra_soundtouch
        )

