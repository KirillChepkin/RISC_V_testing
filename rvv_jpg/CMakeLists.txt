add_library(rvv_jpge STATIC
            src/jpge/jpge.cpp
            src/jpge/jpge_color_transform.cpp
)

target_include_directories(rvv_jpge PUBLIC 
            ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(rvv_jpge PUBLIC
            project_options
            rvv_options
) 