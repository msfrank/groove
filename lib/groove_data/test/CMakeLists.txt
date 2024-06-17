
enable_testing()

# define unit tests

set(TEST_CASES
    category_data_frame_tests.cpp
    category_tests.cpp
    double_data_frame_tests.cpp
    int64_data_frame_tests.cpp
    )

# define test suite driver

add_executable(groove_data_testsuite ${TEST_CASES})
target_include_directories(groove_data_testsuite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_BINARY_DIR}/../src
    )
target_link_libraries(groove_data_testsuite PUBLIC
    groove::groove_data
    gtest::gtest
    )
gtest_discover_tests(groove_data_testsuite)

# define test suite static library

add_library(GrooveDataTestSuite OBJECT ${TEST_CASES})
target_include_directories(GrooveDataTestSuite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_BINARY_DIR}/../src
    )
target_link_libraries(GrooveDataTestSuite PUBLIC
    groove::groove_data
    gtest::gtest
    )