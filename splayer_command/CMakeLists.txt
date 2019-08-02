cmake_minimum_required(VERSION 3.4.1)
project(splayer_command)

set(CMAKE_CXX_STANDARD 14)


if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(DISTRIBUTION ${CMAKE_CURRENT_SOURCE_DIR}/../distribution/mac)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(DISTRIBUTION ${CMAKE_CURRENT_SOURCE_DIR}/../distribution/windows)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(DISTRIBUTION ${CMAKE_CURRENT_SOURCE_DIR}/../distribution/linux)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set(DISTRIBUTION ${CMAKE_CURRENT_SOURCE_DIR}/../distribution/android)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    set(DISTRIBUTION ${CMAKE_CURRENT_SOURCE_DIR}/../distribution/ios)
endif ()

set(SPLAYER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../splayer_core)

add_subdirectory(${SPLAYER_DIR} splayer_core)

aux_source_directory(src src_source)

add_executable(splayer_command ${src_source} src/MacMediaPlayer.cpp src/MacMediaPlayer.h src/MacAOut.cpp src/MacAOut.h src/MacVOut.cpp src/MacVOut.h src/MacPipeline.cpp src/MacPipeline.h src/MacPipelineOpaque.cpp src/MacPipelineOpaque.h src/MacVOutOpaque.cpp src/MacVOutOpaque.h)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${DISTRIBUTION}/ffmpeg/include
        ${SPLAYER_DIR}/include)

target_link_libraries(${PROJECT_NAME}
        splayer_core)