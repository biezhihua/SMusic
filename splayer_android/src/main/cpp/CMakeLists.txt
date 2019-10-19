# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# 设置库名称
project(splayer_andorid)

# 设置cmake最低版本
cmake_minimum_required(VERSION 3.4.1)

# 设置GCC编译器的编译选项
if (CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-declarations -Wmissing-prototypes")
endif (CMAKE_COMPILER_IS_GNUCC)

# 设置clang编译器的编译选项
if (CMAKE_COMPILER_IS_CLANG)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-declarations -Wmissing-prototypes -Wdocumentation -Wunreachable-code")
endif (CMAKE_COMPILER_IS_CLANG)

# android splayer 实现库
add_subdirectory(${CMAKE_SOURCE_DIR}/player/impl)

# android splayer 接口库
add_subdirectory(${CMAKE_SOURCE_DIR}/player/android)