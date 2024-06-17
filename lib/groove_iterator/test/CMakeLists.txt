
enable_testing()

# define unit tests

set(TEST_CASES
    filter_iterator_tests.cpp
    map_iterator_tests.cpp
    )

# define test suite driver

add_executable(groove_iterator_testsuite ${TEST_CASES})
target_link_libraries(groove_iterator_testsuite PUBLIC
    groove::groove_iterator
    gtest::gtest
    )
gtest_discover_tests(groove_iterator_testsuite)

# define test suite static library

add_library(GrooveIteratorTestSuite OBJECT ${TEST_CASES})
target_link_libraries(GrooveIteratorTestSuite PUBLIC
    groove::groove_iterator
    gtest::gtest
    )
