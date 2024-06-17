
enable_testing()

include(GoogleTest)

# define unit tests

set(TEST_CASES
    shard_writer_tests.cpp
    )

# define test suite driver

add_executable(groove_storage_testsuite ${TEST_CASES})
target_link_libraries(groove_storage_testsuite PUBLIC
    groove::groove_storage
    gtest::gtest
    )
gtest_discover_tests(groove_storage_testsuite)

# define test suite static library

add_library(GrooveStorageTestSuite OBJECT ${TEST_CASES})
target_link_libraries(GrooveStorageTestSuite PUBLIC
    groove::groove_storage
    gtest::gtest
    )