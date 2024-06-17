
# define unit tests

set(TEST_CASES
    average_function_tests.cpp
    maximum_function_tests.cpp
    minimum_function_tests.cpp
    samplecount_function_tests.cpp
    sum_function_tests.cpp
    )

# define test suite driver

add_executable(groove_math_testsuite ${TEST_CASES})
target_link_libraries(groove_math_testsuite PUBLIC
    groove::groove_math
    gtest::gtest
    )
gtest_discover_tests(groove_math_testsuite)

# define test suite static library

add_library(GrooveMathTestSuite OBJECT ${TEST_CASES})
target_link_libraries(GrooveMathTestSuite PUBLIC
    groove::groove_math
    gtest::gtest
    )
