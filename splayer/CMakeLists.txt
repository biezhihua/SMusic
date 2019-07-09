cmake_minimum_required(VERSION 3.4.1)

project(splayer_core)

set(CMAKE_CXX_STANDARD 14)

include_directories(../distribution/ffmpeg/include)

include_directories(player)
aux_source_directory(player player)

include_directories(common)
aux_source_directory(common common)

include_directories(soundtouch)
aux_source_directory(soundtouch soundtouch)

add_library(splayer_core
        ${player}
        ${common}
        ${soundtouch}
        )

