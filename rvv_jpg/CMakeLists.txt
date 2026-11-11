add_library(rvv_jpge STATIC
            src/jpge/jpge.cpp
)

target_include_directories(rvv_jpge PUBLIC 
            ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(rvv_jpge PRIVATE project_options)