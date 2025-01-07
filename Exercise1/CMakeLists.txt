set(SOURCES
    src/main.c
    src/ringbuf.c
)

add_executable(exercise1 ${SOURCES})
target_compile_options(exercise1 PRIVATE -Wall -Werror=vla)
target_include_directories(exercise1 PRIVATE include)
