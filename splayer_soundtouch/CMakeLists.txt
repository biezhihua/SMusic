
# 设置cmake最低版本
cmake_minimum_required(VERSION 3.4.1)

# 设置工程名称
project(splayer_soundtouch)

# 指定C++版本
set(CMAKE_CXX_STANDARD 11)

# 添加源码子目录
file(GLOB SOUNDTOUCH_CPP_FILES
        src/*.cpp
        *.cpp)

# 添加头文件子目录
file(GLOB SOUNDTOUCH_H_FILES
        include/*.h
        *.h)

# 设置源文件集合
set(SOUNDTOUCH_SOURCE_FILES ${SOUNDTOUCH_H_FILES} ${SOUNDTOUCH_CPP_FILES})

# 设置库
add_library(${PROJECT_NAME}
        SHARED
        ${SOUNDTOUCH_SOURCE_FILES})

# 链接库头文件
target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR})

