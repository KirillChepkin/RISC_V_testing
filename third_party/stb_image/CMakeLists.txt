
# Build stb_image as a static library.
add_library(stb_image STATIC
    stb_image.c
)

# stb_image is written in C.
set_target_properties(stb_image PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
)

# Make this directory available to targets that link against stb_image.
target_include_directories(stb_image PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
