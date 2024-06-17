
# define unit tests

set(TEST_CASES
    category_int64_indexed_column_tests.cpp
    category_int64_page_tests.cpp
    double_int64_page_tests.cpp
    groove_model_tests.cpp
    indexed_variant_column_tests.cpp
    int64_double_page_tests.cpp
    int64_int64_indexed_column_tests.cpp
    int64_int64_page_tests.cpp
    int64_string_page_tests.cpp
    page_id_tests.cpp
    )

# define test suite driver

add_executable(groove_model_testsuite ${TEST_CASES})
target_link_libraries(groove_model_testsuite PUBLIC
    groove::groove_model
    gtest::gtest
    )
gtest_discover_tests(groove_model_testsuite)

# define test suite static library

add_library(GrooveModelTestSuite OBJECT ${TEST_CASES})
target_link_libraries(GrooveModelTestSuite PUBLIC
    groove::groove_model
    gtest::gtest
    )
