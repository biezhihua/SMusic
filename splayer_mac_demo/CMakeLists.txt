cmake_minimum_required(VERSION 3.4.1)
project(splayer_mac_demo)

set(CMAKE_CXX_STANDARD 14)

set(SPLAYER_CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../splayer_core)
set(SPLAYER_MAC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../splayer_mac)

add_subdirectory(${SPLAYER_MAC_DIR} splayer_mac)

add_executable(splayer_mac_demo main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${SPLAYER_CORE_DIR}/include
        ${SPLAYER_MAC_DIR}/include
        )
target_link_libraries(${PROJECT_NAME} splayer_mac)