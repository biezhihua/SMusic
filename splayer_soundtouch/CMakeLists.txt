cmake_minimum_required(VERSION 3.4.1)
project(splayer_soundtouch)

set(CMAKE_CXX_STANDARD 14)

aux_source_directory(src src_dir)

add_library(${PROJECT_NAME}
        ${src_dir}
        SoundTouchWrapper.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR})

