cmake_minimum_required(VERSION 3.4.1)
project(splayer_command)

set(CMAKE_CXX_STANDARD 14)

set(SPLAYER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../splayer_core)

add_subdirectory(${SPLAYER_DIR} splayer_core)

aux_source_directory(src src_source)

add_executable(splayer_command ${src_source})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${SPLAYER_DIR}/include)

target_link_libraries(${PROJECT_NAME}
        splayer_core)