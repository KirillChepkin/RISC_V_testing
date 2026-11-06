
cmake_minimum_required(VERSION 3.25)

project(RiscvJpegCompressor
    VERSION 1.0
    DESCRIPTION "JPEG compressor and decompressor using RISC-V vector intrinsics"
    LANGUAGES C CXX
)

# ============================================================
# 1. Language configuration
# ============================================================

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)


# ============================================================
# 2. Build options
# ============================================================

option(BUILD_REFERENCE
    "Build the original JPEG implementation"
    ON
)

option(BUILD_RVV_JPG
    "Build the new RVV JPEG implementation"
    OFF
)

option(BUILD_EXAMPLES
    "Build example applications"
    ON
)

option(BUILD_TESTS
    "Build automated tests"
    ON
)

option(ENABLE_RVV
    "Enable RISC-V vector architecture flags"
    OFF
)


# ============================================================
# 3. Common compiler configuration
# ============================================================

add_library(project_options INTERFACE)

target_compile_options(project_options INTERFACE
    -Wall
    -Wextra
)


# ============================================================
# 4. Third-party libraries
# ============================================================

add_library(rvv_options INTERFACE)


# ============================================================
# 5. JPEG implementations
# ============================================================

if(BUILD_REFERENCE)
    add_subdirectory(reference)
endif()

if(BUILD_RVV_JPG)
    add_subdirectory(rvv_jpg)
endif()


# ============================================================
# 6. Examples
# ============================================================

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()


# ============================================================
# 7. Tests
# ============================================================

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
