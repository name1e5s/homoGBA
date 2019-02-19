cmake_minimum_required(VERSION 3.10)
project(homoGBA)

enable_testing()



set(CMAKE_C_STANDARD 11)

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/out)

set(SOURCE_FILES main.c)

add_executable(homo ${SOURCE_FILES})

include_directories(include)
include_directories(tools/log/log.c/src)


add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(tools)

target_link_libraries(homo
    ${CMAKE_PROJECT_NAME}_lib
        log)
