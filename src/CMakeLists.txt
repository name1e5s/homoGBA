project(${CMAKE_PROJECT_NAME}_lib)

set(HEADER_FILES)
set(SOURCE_FILES
        version.c
        cartridge.c
        memory.c
        gba.c
        cpu.c
        cpu-arm.c
        cpu-thumb.c
        apu.c
        ppu.c
        timer.c
        int.c)

#The library contains header and source files.
add_library(${CMAKE_PROJECT_NAME}_lib STATIC ${SOURCE_FILES}
        ${HEADER_FILES})
