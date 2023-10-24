project(lab_test LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG       v1.14.0
)
FetchContent_MakeAvailable(googletest)

##
file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/src/storage/**.c)
file(GLOB_RECURSE HEADERS ${CMAKE_SOURCE_DIR}/include/**.h)
file(GLOB_RECURSE TEST_SOURCES ${CMAKE_CURRENT_LIST_DIR}/**.cpp)
file(GLOB_RECURSE TEST_HEADERS ${CMAKE_CURRENT_LIST_DIR}/**.h)

## google test
enable_testing()

add_executable(
        ${PROJECT_NAME}
        ${SOURCES}
        ${HEADERS}
        ${TEST_SOURCES}
        ${TEST_HEADERS}
)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(
        ${PROJECT_NAME}
        GTest::gtest_main
        GTest::gmock_main
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})