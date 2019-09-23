cmake_minimum_required(VERSION 3.4.1)
project(splayer_mac_demo)

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

# 设置引擎目录
set(SPLAYER_ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../splayer_engine)

# 设置SDL目录
set(SPLAYER_SDL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../splayer_sdl)

# 设置SDL2包
set(SDL2_FOUND TRUE)
find_package(SDL2 REQUIRED)

add_subdirectory(${SPLAYER_SDL_DIR} alias_splayer_sdl)

add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${DISTRIBUTION_DIR}/ffmpeg/include
        ${SPLAYER_SOUND_TOUCH_DIR}/include
        ${SPLAYER_SOUND_TOUCH_DIR}
        ${SDL2_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${SPLAYER_SDL_DIR}/include
        ${SPLAYER_ENGINE_DIR}/include
        )
target_link_libraries(${PROJECT_NAME} splayer_sdl)

# 链接FFmpeg模块
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    file(GLOB libs "${DISTRIBUTION_DIR}/ffmpeg/lib/*.dylib")
    target_link_libraries(${PROJECT_NAME} ${libs})
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
endif ()