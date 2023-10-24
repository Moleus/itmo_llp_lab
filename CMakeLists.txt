cmake_minimum_required(VERSION 3.26)
project(lab LANGUAGES C)

set(CMAKE_C_STANDARD 11)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -g")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_VERBOSE_MAKEFILE ON)

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/**.c)
file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_LIST_DIR}/include/**.h)

#windows compatability setup
IF ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    set(WINDOWS TRUE)
    add_compile_definitions(WINDOWS)
ENDIF ()

include_directories(include)

#target_sources(${PROJECT_NAME} PRIVATE ${SOURCES} PUBLIC ${HEADERS})
#
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# add cmake from test/ path
add_subdirectory(test)

add_subdirectory(benchmark)