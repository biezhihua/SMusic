
# 设置cmake最低版本
cmake_minimum_required(VERSION 3.4.1)

# 设置库名称
project(splayer_sdl)

# 设置C++版本
set(CMAKE_CXX_STANDARD 11)

# 设置根目录
get_filename_component(SDL_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ ABSOLUTE)

# 设置FFMPEG头文件目录
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(SDL_DISTRIBUTION_DIR ${SDL_ROOT_DIR}/distribution/macos)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(SDL_DISTRIBUTION_DIR ${SDL_ROOT_DIR}/distribution/windows)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(SDL_DISTRIBUTION_DIR ${SDL_ROOT_DIR}/distribution/linux)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set(SDL_DISTRIBUTION_DIR ${SDL_ROOT_DIR}/distribution/android)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    set(SDL_DISTRIBUTION_DIR ${SDL_ROOT_DIR}/distribution/ios)
endif ()


#
message("LOG SDL SDL_ROOT_DIR = ${SDL_ROOT_DIR}")
message("LOG SDL SDL_DISTRIBUTION_DIR = ${SDL_DISTRIBUTION_DIR}")
message("LOG SDL CMAKE_SYSTEM_NAME = ${CMAKE_SYSTEM_NAME}")

# 设置SDL2包
set(SDL2_FOUND TRUE)
find_package(SDL2 REQUIRED)

# 添加引擎子模块目录
add_subdirectory(${SDL_ROOT_DIR}/splayer_engine splayer_engine)

aux_source_directory(src src_source)

add_library(splayer_sdl SHARED ${src_source})

target_include_directories(${PROJECT_NAME} PRIVATE
        # 引入FFmpeg头文件
        ${SDL_DISTRIBUTION_DIR}/ffmpeg/include

        # 引入SoundTouch头文件
        ${SDL_ROOT_DIR}/splayer_soundtouch/include
        ${SDL_ROOT_DIR}/splayer_soundtouch

        # 引入引擎头文件
        ${SDL_ROOT_DIR}/splayer_engine/include
        ${SDL_ROOT_DIR}/splayer_engine

        # 引入自有代码头文件
        ${CMAKE_CURRENT_SOURCE_DIR}/include

        # 引入SDL2头文件
        ${SDL2_INCLUDE_DIRS}

        )

# 链接子模块
target_link_libraries(${PROJECT_NAME}
        splayer_engine
        ${SDL2_LIBRARIES}
        )

# 链接FFmpeg模块
#if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
#    file(GLOB libs "${SDL_DISTRIBUTION_DIR}/ffmpeg/lib/*.dylib")
#    target_link_libraries(${PROJECT_NAME} ${libs})
#elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
#elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
#elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")
#elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
#endif ()