cmake_minimum_required(VERSION 3.4.1)
project(splayer_sdl)

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

set(SPLAYER_ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../splayer_engine)

add_subdirectory(${SPLAYER_ENGINE_DIR} splayer_engine)

aux_source_directory(src src_source)

set(SDL2_FOUND TRUE)
find_package(SDL2 REQUIRED)

add_library(splayer_sdl ${src_source})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${DISTRIBUTION_DIR}/ffmpeg/include
        ${SPLAYER_ENGINE_DIR}/include
        ${SDL2_INCLUDE_DIRS}
        )

target_link_libraries(${PROJECT_NAME} splayer_engine ${SDL2_LIBRARIES})

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    file(GLOB libs "${DISTRIBUTION_DIR}/ffmpeg/lib/*.dylib")
    target_link_libraries(${PROJECT_NAME} ${libs})
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Android")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "iOS")
endif ()