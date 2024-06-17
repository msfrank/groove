
# define unit tests

set(TEST_CASES
    bar_group_shape_tests.cpp
    series_group_shape_tests.cpp
    )

# define test suite driver

add_executable(groove_shapes_testsuite ${TEST_CASES})
target_link_libraries(groove_shapes_testsuite PUBLIC
    groove::groove_shapes
    gtest::gtest
    )
gtest_discover_tests(groove_shapes_testsuite)

# define test suite static library

add_library(GrooveShapesTestSuite OBJECT ${TEST_CASES})
target_link_libraries(GrooveShapesTestSuite PUBLIC
    groove::groove_shapes
    gtest::gtest
    )