add_executable(
    unit_tests
    test_jpge_color_transform.cpp)

target_link_libraries(
    unit_tests PRIVATE
    rvv_jpge
)