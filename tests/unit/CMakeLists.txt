add_executable(
    unit_tests
    test_jpge_color_transform.cpp)

target_link_libraries(
    unit_tests PRIVATE
    rvv_jpge
    # GTest
)

add_test(
    NAME jpge_color_transform
    COMMAND unit_tests
)

# include(GoogleTest)
# gtest_discover_tests(unit_tests)