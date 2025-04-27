# Tests ########################################################################

message(STATUS "### Configuring Tests ###")

project(TestBench)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

enable_testing()

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
    TestRunner.cc
)

target_link_libraries(${PROJECT_NAME} PUBLIC 
    Source
    GTest::gtest
)

set(TEST_AUDIO_DIR "${PROJECT_ROOT}/tests/test files/")
target_compile_definitions(${PROJECT_NAME} PUBLIC
    TEST_AUDIO_DIR="${TEST_AUDIO_DIR}"
)

target_compile_definitions(mp3gain PUBLIC EXCLUDE_MAIN)

message(STATUS "Test Audio Directory is set to ${TEST_AUDIO_DIR}")

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})
