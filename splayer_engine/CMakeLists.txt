cmake_minimum_required(VERSION 3.4.1)

project(splayer_engine)

set(CMAKE_CXX_STANDARD 14)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(DISTRIBUTION_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../distribution/macos)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(DISTRIBUTION_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../distribution/windows)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(DISTRIBUTION_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../distribution/linux)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set(DISTRIBUTION_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../distribution/android)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    set(DISTRIBUTION_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../distribution/ios)
endif ()

aux_source_directory(src/common common)
aux_source_directory(src/convertor convertor)
aux_source_directory(src/decoder decoder)
aux_source_directory(src/device device)
aux_source_directory(src/player player)
aux_source_directory(src/queue queue)
aux_source_directory(src/renderer renderer)
aux_source_directory(src/sync sync)
aux_source_directory(src/message message)
aux_source_directory(src/stream stream)

add_library(${PROJECT_NAME}
        ${common}
        ${convertor}
        ${decoder}
        ${device}
        ${player}
        ${queue}
        ${renderer}
        ${sync}
        ${message}
        ${stream}
        )

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${DISTRIBUTION_DIR}/ffmpeg/include)
