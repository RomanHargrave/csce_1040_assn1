cmake_minimum_required(VERSION 2.8.4)
project(homework_01)

add_definitions(-D__USE_POSIX)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -lm")

set(SOURCE_FILES
    src/util.h
    src/util.c
    src/tui.h
    src/tui.c
    src/grading.h
    src/grading.c
    src/shell/options.h
    src/shell/shell_ui.c
    src/shell/print_gradebook.c
    src/shell/model_display.h
    src/shell/model_display.c
    src/models/models.h
    src/models/models.c
    src/models/model_io.h
    src/models/model_io.c)

add_executable(test_serialize ${SOURCE_FILES} src/test_serialize.c)
add_executable(gradebook ${SOURCE_FILES} src/shell.c)