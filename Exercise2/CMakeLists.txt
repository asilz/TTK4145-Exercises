set(SOURCES
    src/main.c
)

add_executable(exercise2 ${SOURCES})
target_compile_options(exercise2 PRIVATE -Wall -Werror=vla)
target_include_directories(exercise2 PRIVATE include)
