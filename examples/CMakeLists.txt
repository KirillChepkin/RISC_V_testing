
# ============================================================
# Photo compressor
# ============================================================

add_executable(photo_compressor
    photo_compressor.cpp
)

target_link_libraries(photo_compressor PRIVATE
    reference_jpge
    stb_image
    project_options
)


# ============================================================
# Photo decompressor
# ============================================================

add_executable(photo_decompressor
    photo_decompressor.cpp
)

target_link_libraries(photo_decompressor PRIVATE
    reference_jpgd
    stb_image
    project_options
)


# ============================================================
# Scalar intrinsics example
# ============================================================

add_executable(intrinsics_test_scalar
    intrinsics.cpp
)

target_link_libraries(intrinsics_test_scalar PRIVATE
    project_options
)


# ============================================================
# Vector intrinsics example
# ============================================================

if(ENABLE_RVV)

    add_executable(intrinsics_test_vector
        intrinsics.cpp
    )

    target_compile_definitions(intrinsics_test_vector PRIVATE
        VECTOR
    )

    target_link_libraries(intrinsics_test_vector PRIVATE
        project_options
        rvv_options
    )

    target_link_libraries(intrinsics_test_vector PRIVATE
        project_options
    )

endif()


# ============================================================
# Generate RISC-V assembly
# ============================================================

if(ENABLE_RVV)

    set(ASSEMBLY_DIR
        ${CMAKE_BINARY_DIR}/riscv_asm
    )

    add_custom_command(
        OUTPUT
            ${ASSEMBLY_DIR}/intrinsics_for_vector.asm

        COMMAND
            ${CMAKE_COMMAND} -E make_directory ${ASSEMBLY_DIR}

        COMMAND
            ${CMAKE_CXX_COMPILER}
            -std=gnu++20
            -march=rv64gcv
            -DVECTOR
            -S
            ${CMAKE_CURRENT_SOURCE_DIR}/intrinsics.cpp
            -o ${ASSEMBLY_DIR}/intrinsics_for_vector.asm

        DEPENDS
            intrinsics.cpp

        VERBATIM
    )


    add_custom_command(
        OUTPUT
            ${ASSEMBLY_DIR}/intrinsics_for_general.asm

        COMMAND
            ${CMAKE_COMMAND} -E make_directory ${ASSEMBLY_DIR}

        COMMAND
            ${CMAKE_CXX_COMPILER}
            -std=gnu++20
            -march=rv64gcv
            -S
            ${CMAKE_CURRENT_SOURCE_DIR}/intrinsics.cpp
            -o ${ASSEMBLY_DIR}/intrinsics_for_general.asm

        DEPENDS
            intrinsics.cpp

        VERBATIM
    )


    add_custom_target(intrinsics_assembly
        DEPENDS
            ${ASSEMBLY_DIR}/intrinsics_for_vector.asm
            ${ASSEMBLY_DIR}/intrinsics_for_general.asm
    )

endif()


# ============================================================
# Performance measurement targets (Linux)
# ============================================================

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")

    find_program(PERF_EXECUTABLE perf)

    if(PERF_EXECUTABLE)

        add_custom_target(comp_photo
            COMMAND
                ${PERF_EXECUTABLE} stat -e cycles
                $<TARGET_FILE:photo_compressor>

            DEPENDS photo_compressor
            USES_TERMINAL
        )


        add_custom_target(decomp_photo
            COMMAND
                ${PERF_EXECUTABLE} stat -e cycles
                $<TARGET_FILE:photo_decompressor>

            DEPENDS photo_decompressor
            USES_TERMINAL
        )


        if(ENABLE_RVV)

            add_custom_target(intrinsics
                COMMAND
                    ${PERF_EXECUTABLE} stat -e cycles
                    $<TARGET_FILE:intrinsics_test_vector>

                COMMAND
                    ${PERF_EXECUTABLE} stat -e cycles
                    $<TARGET_FILE:intrinsics_test_scalar>

                DEPENDS
                    intrinsics_test_vector
                    intrinsics_test_scalar

                USES_TERMINAL
            )

        endif()

    endif()

endif()
