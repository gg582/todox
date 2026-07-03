cmake_minimum_required(VERSION 3.10)
project(todox C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

find_package(PkgConfig REQUIRED)
pkg_check_modules(DBUS REQUIRED dbus-1)
include_directories(${DBUS_INCLUDE_DIRS})
link_directories(${DBUS_LIBRARY_DIRS})
add_compile_options(${DBUS_CFLAGS_OTHER})

set(TODOX_DEFAULT_ALARM_FILE "/usr/local/etc/todox/alarm.txt")
add_compile_definitions(TODOX_DEFAULT_ALARM_FILE="${TODOX_DEFAULT_ALARM_FILE}")

set(TODOX_SOURCES
    src/main.c
    src/app/app.c
    src/app/parse.c
    src/app/io.c
    src/notify/notify.c
    src/notify/dbus.c
    src/error/error.c
    src/list/list.c
    src/time/time.c
    src/time/table.c
    src/repeat/repeat.c
)

add_executable(todox ${TODOX_SOURCES})

target_link_libraries(todox ${DBUS_LIBRARIES})

enable_testing()

set(TODOX_TEST_SOURCES
    tests/main.c
    tests/test_list.c
    tests/test_parse.c
    tests/test_time.c
    tests/test_repeat.c
    src/app/parse.c
    src/error/error.c
    src/list/list.c
    src/time/time.c
    src/repeat/repeat.c
)

add_executable(todox_test ${TODOX_TEST_SOURCES})
target_link_libraries(todox_test ${DBUS_LIBRARIES})
add_test(NAME todox_test COMMAND todox_test)

# Add custom target for clang-format
find_program(CLANG_FORMAT "clang-format")
if(CLANG_FORMAT)
    file(GLOB_RECURSE ALL_SOURCE_FILES
        ${PROJECT_SOURCE_DIR}/src/*.c
        ${PROJECT_SOURCE_DIR}/include/*.h
        ${PROJECT_SOURCE_DIR}/tests/*.c
    )
    add_custom_target(format
        COMMAND ${CLANG_FORMAT} -i ${ALL_SOURCE_FILES}
        COMMENT "Formatting C source and header files with clang-format"
    )
endif()

