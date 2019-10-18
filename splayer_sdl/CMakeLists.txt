cmake_minimum_required(VERSION 3.4.1)
project(splayer_sdl)

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

# 设置引擎目录
set(SPLAYER_ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../splayer_engine)

# 设置SoundTouch目录
set(SPLAYER_SOUND_TOUCH_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../splayer_soundtouch)

# 设置SDL2包
set(SDL2_FOUND TRUE)
find_package(SDL2 REQUIRED)

# 添加引擎子模块目录
add_subdirectory(${SPLAYER_ENGINE_DIR} alias_splayer_engine)

aux_source_directory(src src_source)

add_library(splayer_sdl ${src_source})

target_include_directories(${PROJECT_NAME} PRIVATE
        # 引入FFmpeg头文件
        ${DISTRIBUTION_DIR}/ffmpeg/include

        # 引入SoundTouch头文件
        ${SPLAYER_SOUND_TOUCH_DIR}/include
        ${SPLAYER_SOUND_TOUCH_DIR}

        # 引入引擎头文件
        ${SPLAYER_ENGINE_DIR}/include

        # 引入自有代码头文件
        ${CMAKE_CURRENT_SOURCE_DIR}/include

        # 引入SDL2头文件
        ${SDL2_INCLUDE_DIRS}

        )

# 链接子模块
target_link_libraries(${PROJECT_NAME} splayer_engine ${SDL2_LIBRARIES})

# 链接FFmpeg模块
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    file(GLOB libs "${DISTRIBUTION_DIR}/ffmpeg/lib/*.dylib")
    target_link_libraries(${PROJECT_NAME} ${libs})
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
endif ()