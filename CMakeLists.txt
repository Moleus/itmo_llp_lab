cmake_minimum_required(VERSION 3.26)
project(lab C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -g")

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/**.c)
file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_LIST_DIR}/include/**.h)
file(GLOB_RECURSE TEST_SOURCES ${CMAKE_CURRENT_LIST_DIR}/test/**.c)

include_directories(include)

add_executable(${PROJECT_NAME} ${TEST_SOURCES})

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES} PUBLIC ${HEADERS})

add_library(${PROJECT_NAME}_lib ${SOURCES})