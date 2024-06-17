
# define unit tests

set(TEST_CASES
    storage_supervisor_tests.cpp
    )

# define test suite driver

add_executable(groove_agent_testsuite ${TEST_CASES})
target_link_libraries(groove_agent_testsuite PUBLIC
    GrooveAgentRuntime
    gtest::gtest
    )
gtest_discover_tests(groove_agent_testsuite)

# define test suite static library

add_library(GrooveAgentTestSuite OBJECT ${TEST_CASES})
target_link_libraries(GrooveAgentTestSuite PUBLIC
    GrooveAgentRuntime
    gtest::gtest
)
