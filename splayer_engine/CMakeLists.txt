cmake_minimum_required(VERSION 3.4.1)

project(splayer_engine)

set(CMAKE_CXX_STANDARD 14)

# 设置FFMPEG头文件目录
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

# 设置SoundTouch目录
set(SPLAYER_SOUND_TOUCH_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../splayer_soundtouch)

# 添加SoundTouch子模块目录
add_subdirectory(${SPLAYER_SOUND_TOUCH_DIR} alias_splayer_soundtouch)

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
aux_source_directory(src/tools tools)

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
        ${tools}
        )

target_include_directories(${PROJECT_NAME} PRIVATE
        # 引入FFmpeg头文件
        ${DISTRIBUTION_DIR}/ffmpeg/include

        # 引入SoundTouch头文件
        ${SPLAYER_SOUND_TOUCH_DIR}/include
        ${SPLAYER_SOUND_TOUCH_DIR}

        # 引入自有代码头文件
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        )

# 链接子模块
target_link_libraries(${PROJECT_NAME} splayer_soundtouch)

# 链接FFmpeg模块
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    file(GLOB libs "${DISTRIBUTION_DIR}/ffmpeg/lib/*.dylib")
    target_link_libraries(${PROJECT_NAME} ${libs})
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
endif ()