cmake_minimum_required(VERSION 3.16)
project(lab_benchmark LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/src/storage/**.c ${CMAKE_SOURCE_DIR}/src/document_db/**.c)
file(GLOB_RECURSE HEADERS ${CMAKE_SOURCE_DIR}/include/**.h)
file(GLOB_RECURSE TEST_SOURCES ${CMAKE_CURRENT_LIST_DIR}/**.c)

add_executable(
        ${PROJECT_NAME}
        ${SOURCES}
        ${HEADERS}
        ${TEST_SOURCES}
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)

#target_link_libraries(
#        ${PROJECT_NAME}
#)