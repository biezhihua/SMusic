cmake_minimum_required(VERSION 3.4.1)
project(splayer_mac_example)

set(CMAKE_CXX_STANDARD 11)

get_filename_component(MAC_ROOT_DIR ${CMAKE_SOURCE_DIR}/../../ ABSOLUTE)

# 设置FFMPEG头文件目录
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MAC_DISTRIBUTION_DIR ${MAC_ROOT_DIR}/distribution/macos)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(MAC_DISTRIBUTION_DIR ${MAC_ROOT_DIR}/distribution/windows)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(MAC_DISTRIBUTION_DIR ${MAC_ROOT_DIR}/distribution/linux)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set(MAC_DISTRIBUTION_DIR ${MAC_ROOT_DIR}/distribution/android)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    set(MAC_DISTRIBUTION_DIR ${MAC_ROOT_DIR}/distribution/ios)
endif ()

# 设置SoundTouch目录
set(SPLAYER_SOUND_TOUCH_DIR ${MAC_ROOT_DIR}/splayer_soundtouch)

# 设置引擎目录
set(SPLAYER_ENGINE_DIR ${MAC_ROOT_DIR}/splayer_engine)

# 设置SDL目录
set(SPLAYER_SDL_DIR ${MAC_ROOT_DIR}/splayer_sdl)

# 设置SDL2包
set(SDL2_FOUND TRUE)
find_package(SDL2 REQUIRED)

# 消息
message("LOG ENGINE MAC_ROOT_DIR = ${MAC_ROOT_DIR}")
message("LOG ENGINE MAC_DISTRIBUTION_DIR = ${MAC_DISTRIBUTION_DIR}")
message("LOG ENGINE SPLAYER_SOUND_TOUCH_DIR = ${SPLAYER_SOUND_TOUCH_DIR}")
message("LOG ENGINE CMAKE_SYSTEM_NAME = ${CMAKE_SYSTEM_NAME}")

add_subdirectory(${SPLAYER_SDL_DIR} splayer_sdl)

add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${MAC_DISTRIBUTION_DIR}/ffmpeg/include
        ${SPLAYER_SOUND_TOUCH_DIR}/include
        ${SPLAYER_SOUND_TOUCH_DIR}
        ${SDL2_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${SPLAYER_SDL_DIR}/include
        ${SPLAYER_ENGINE_DIR}/include
        )
target_link_libraries(${PROJECT_NAME} splayer_sdl)
