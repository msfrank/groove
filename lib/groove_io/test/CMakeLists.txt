
# define unit tests

set(TEST_CASES
    dataset_reader_tests.cpp
    dataset_writer_tests.cpp
    )

# define test suite driver

add_executable(groove_io_testsuite ${TEST_CASES})
target_link_libraries(groove_io_testsuite PUBLIC
    groove::groove_io
    gtest::gtest
    )
gtest_discover_tests(groove_io_testsuite)

# define test suite static library

add_library(GrooveIOTestSuite OBJECT ${TEST_CASES})
target_link_libraries(GrooveIOTestSuite PUBLIC
    groove::groove_io
    gtest::gtest
    )