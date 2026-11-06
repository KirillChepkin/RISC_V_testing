# ============================================================
# Original JPEG compression library
# ============================================================

add_library(reference_jpge STATIC

    src/jpge/jpge.cpp

    src/jpge/jpge_color_transform.cpp

    src/jpge/jpge_forward_dct.cpp

    src/jpge/jpge_huffman_tables.cpp

    src/jpge/jpge_marker_writer.cpp
)


# ============================================================
# Original JPEG decompression library
# ============================================================

add_library(reference_jpgd STATIC

    src/jpgd/jpgd.cpp

    src/jpgd/icdt.cpp

    src/jpgd/color_converter.cpp

    src/jpgd/huffman_decoder.cpp
)


# ============================================================
# Public header directories
# ============================================================

target_include_directories(reference_jpge PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(reference_jpgd PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)


# ============================================================
# Compiler configuration
# ============================================================

target_link_libraries(reference_jpge PRIVATE
    project_options
)

target_link_libraries(reference_jpgd PRIVATE
    project_options
)
