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

add_executable(todox
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
)

target_link_libraries(todox ${DBUS_LIBRARIES})
