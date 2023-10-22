cmake_minimum_required(VERSION 3.26)
project(lab_benchmark C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_LIST_DIR}/**.c)
#file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_LIST_DIR}/**.h)

add_executable(${PROJECT_NAME} ${SOURCES})