
# 设置cmake最低版本
cmake_minimum_required(VERSION 3.4.1)

# 设置库名称
project(splayer_engine)

# 指定C++版本
set(CMAKE_CXX_STANDARD 11)

## 设置GCC编译器的编译选项
#if (CMAKE_COMPILER_IS_GNUCC)
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-declarations -Wmissing-prototypes")
#endif (CMAKE_COMPILER_IS_GNUCC)
#
## 设置clang编译器的编译选项
#if (CMAKE_COMPILER_IS_CLANG)
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-declarations -Wmissing-prototypes -Wdocumentation -Wunreachable-code")
#endif (CMAKE_COMPILER_IS_CLANG)

# 设置根目录
get_filename_component(ENGINE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ ABSOLUTE)

# 设置FFMPEG头文件目录
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(ENGINE_DISTRIBUTION_DIR ${ENGINE_ROOT_DIR}/distribution/macos)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(ENGINE_DISTRIBUTION_DIR ${ENGINE_ROOT_DIR}/distribution/windows)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(ENGINE_DISTRIBUTION_DIR ${ENGINE_ROOT_DIR}/distribution/linux)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    set(ENGINE_DISTRIBUTION_DIR ${ENGINE_ROOT_DIR}/distribution/android)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    set(ENGINE_DISTRIBUTION_DIR ${ENGINE_ROOT_DIR}/distribution/ios)
endif ()

# 设置SoundTouch目录
set(SPLAYER_SOUND_TOUCH_DIR ${ENGINE_ROOT_DIR}/splayer_soundtouch)

# 打印消息
message("LOG ENGINE ENGINE_ROOT_DIR = ${ENGINE_ROOT_DIR}")
message("LOG ENGINE ENGINE_DISTRIBUTION_DIR = ${ENGINE_DISTRIBUTION_DIR}")
message("LOG ENGINE CMAKE_SYSTEM_NAME = ${CMAKE_SYSTEM_NAME}")

# 添加SoundTouch子模块目录
add_subdirectory(${ENGINE_ROOT_DIR}/splayer_soundtouch splayer_soundtouch)

# 设置子文件夹源码依赖
# https://discourse.urho3d.io/t/how-do-you-include-source-in-subdirectories/871

# 添加源码子目录
file(GLOB ENGINE_CPP_FILES
        src/*/*.cpp
        *.cpp
        )

# 添加头文件子目录
file(GLOB ENGINE_H_FILES
        include/*/*.h
        *.h
        )

# 设置源文件集合
set(ENGINE_SOURCE_FILES ${ENGINE_CPP_FILES} ${ENGINE_H_FILES})

# 设置库
add_library(${PROJECT_NAME} SHARED ${ENGINE_SOURCE_FILES})

# 链接头文件
target_include_directories(${PROJECT_NAME} PRIVATE
        # 引入FFmpeg头文件
        ${ENGINE_DISTRIBUTION_DIR}/ffmpeg/include

        # 引入SoundTouch头文件
        ${ENGINE_ROOT_DIR}/splayer_soundtouch/include
        ${ENGINE_ROOT_DIR}/splayer_soundtouch

        # 引入自有代码头文件
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include/common
        ${CMAKE_CURRENT_SOURCE_DIR}/include/convertor
        ${CMAKE_CURRENT_SOURCE_DIR}/include/decoder
        ${CMAKE_CURRENT_SOURCE_DIR}/include/device
        ${CMAKE_CURRENT_SOURCE_DIR}/include/message
        ${CMAKE_CURRENT_SOURCE_DIR}/include/player
        ${CMAKE_CURRENT_SOURCE_DIR}/include/queue
        ${CMAKE_CURRENT_SOURCE_DIR}/include/renderer
        ${CMAKE_CURRENT_SOURCE_DIR}/include/stream
        ${CMAKE_CURRENT_SOURCE_DIR}/include/sync
        )

# 链接子模块
target_link_libraries(${PROJECT_NAME} splayer_soundtouch)

# 链接FFmpeg模块
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    file(GLOB ffmpegLibs "${ENGINE_DISTRIBUTION_DIR}/ffmpeg/lib/*.dylib")
    target_link_libraries(${PROJECT_NAME} ${ffmpegLibs} ${openglesLibs})
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")
    target_link_libraries(${PROJECT_NAME}
            splayer_ffmpeg
            splayer_openssl_crypto
            splayer_openssl_ssl
            -landroid
            -llog
            )
elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
endif ()