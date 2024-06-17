enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    test_unit_dimension.cpp
    test_unit_reference_system.cpp
    )

# define test suite driver

add_executable(groove_units_testsuite ${TEST_CASES})
target_include_directories(groove_units_testsuite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_BINARY_DIR}/../src
    )
target_link_libraries(groove_units_testsuite PUBLIC
    groove::groove_units
    gtest::gtest
    )
gtest_discover_tests(groove_units_testsuite)

# define test suite static library

add_library(GrooveUnitsTestSuite OBJECT ${TEST_CASES})
target_include_directories(GrooveUnitsTestSuite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_BINARY_DIR}/../src
    )
target_link_libraries(GrooveUnitsTestSuite PUBLIC
    groove::groove_units
    gtest::gtest
    )