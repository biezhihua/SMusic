
# 设置cmake最低版本
cmake_minimum_required(VERSION 3.4.1)

# 设置工程名称
project(splayer_soundtouch)

# 指定C++版本
set(CMAKE_CXX_STANDARD 11)

# 设置GCC编译器的编译选项
if (CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-declarations -Wmissing-prototypes")
endif (CMAKE_COMPILER_IS_GNUCC)

# 设置clang编译器的编译选项
if (CMAKE_COMPILER_IS_CLANG)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-declarations -Wmissing-prototypes -Wdocumentation -Wunreachable-code")
endif (CMAKE_COMPILER_IS_CLANG)

# 设置源码目录
aux_source_directory(src src_dir)

# 设置库
add_library(${PROJECT_NAME}
        ${src_dir}
        SoundTouchWrapper.cpp)

# 链接库头文件
target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR})

