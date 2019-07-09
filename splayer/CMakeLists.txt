cmake_minimum_required(VERSION 3.4.1)

project(splayer_core)

set(CMAKE_CXX_STANDARD 14)

include_directories(../distribution/ffmpeg/include)

include_directories(player)
aux_source_directory(player player)

include_directories(sdl)
aux_source_directory(sdl sdl)

include_directories(soundtouch)
aux_source_directory(soundtouch soundtouch)

add_library(splayer_core
        ${player}
        ${sdl}
        ${soundtouch}
        )

