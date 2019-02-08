project(${CMAKE_PROJECT_NAME}_lib)

set(HEADER_FILES
)
set(SOURCE_FILES
    version.c
        gba.c cartridge.c)

# The library contains header and source files.
add_library(${CMAKE_PROJECT_NAME}_lib STATIC
    ${SOURCE_FILES}
    ${HEADER_FILES}
)
