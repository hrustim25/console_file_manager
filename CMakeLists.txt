cmake_minimum_required(VERSION 3.12)

project(file-manager)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

set(CMAKE_C_STANDARD 99)
set(C_STANDARD_REQUIRED TRUE)

add_library(files file_handler.c)

add_executable(manager main.c)
target_link_libraries(manager files ${CURSES_LIBRARIES})