project(homolog)

set(SOURCE_FILES homolog.c)

add_library(homolog STATIC
        ${SOURCE_FILES})