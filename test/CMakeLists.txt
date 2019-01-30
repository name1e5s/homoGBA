#project(${CMAKE_PROJECT_NAME}_tests)

# Download Google Test
set(GOOGLETEST_DIR ${CMAKE_SOURCE_DIR}/test/lib/googletest)
if(NOT EXISTS ${GOOGLETEST_DIR})
    find_package(Git REQUIRED)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} clone https://github.com/google/googletest ${GOOGLETEST_DIR} --depth=1
    )
endif()

add_subdirectory(lib/googletest)
add_subdirectory(unit_tests)
