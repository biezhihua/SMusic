cmake_minimum_required(VERSION 3.4.1)
project(splayer_mac_demo)

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


set(SPLAYER_CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../splayer_core)
set(SPLAYER_MAC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../splayer_mac)

add_subdirectory(${SPLAYER_MAC_DIR} splayer_mac)

set(SDL2_FOUND TRUE)
find_package(SDL2 REQUIRED)

add_executable(splayer_mac_demo main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${SPLAYER_CORE_DIR}/include
        ${SPLAYER_MAC_DIR}/include
        ${DISTRIBUTION_DIR}/ffmpeg/include
        ${SDL2_INCLUDE_DIRS}
        )
target_link_libraries(${PROJECT_NAME} splayer_mac)