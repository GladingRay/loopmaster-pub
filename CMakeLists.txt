# cmd args
# CMAKE_BUILD_TYPE: [cmake]
# BUILD_ARCH: [sw6a/sw8a/skylakex]
# ENABLE_ASAN: [ON/OFF]
# ENABLE_TEST: [ON/OFF]
# BLAS_LIB: path or libs

cmake_minimum_required(VERSION 3.7)

project(LOOP_MASTER VERSION 1.0 LANGUAGES CXX C ASM)

set(CMAKE_CXX_STANDARD 14)

if (EXISTS ${CMAKE_CURRENT_BINARY_DIR}/config.cmake)
    message(STATUS "Build with config.cmake")
    include(${CMAKE_CURRENT_BINARY_DIR}/config.cmake)
else()
    message(WARNING "Build without config.cmake, please check cmdargs")
endif()

if (${CMAKE_BUILD_TYPE} STREQUAL Debug)
    message(STATUS "Build in Debug mode")
else()
    message(STATUS "Build in Release mode")
endif()

if (${BUILD_ARCH} STREQUAL sw6a)
    set(ENABLE_ASAN OFF)
endif()
message(STATUS "Build on ${BUILD_ARCH} with BLAS: ${BLAS_LIB}")

if (${ENABLE_ASAN})
    message(STATUS "Build with ASAN")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g)
    if (${CMAKE_VERSION} VERSION_LESS 3.13)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
    else()
        add_link_options(-fsanitize=address)
    endif()
endif()

include_directories(./include/)

file(GLOB GemmFamilyGeneratorSrc src/*.cc)

set(ASTSrc src/expr.cc src/stmt.cc src/check_end.cc src/check_target.cc
            src/walk_find_node.cc src/node_copy.cc src/walk_find_all_nodes.cc
            src/ast_helpers.cc)
set(GemmGeneratorSrc ${ASTSrc} src/scheme_info.cc src/gemm_generator.cc
                        src/pack_generator.cc src/wcpack_generator.cc)
set(ParallelSrc ${GemmGeneratorSrc} src/parallel_scheme.cc src/pass.cc)

if (${ENABLE_TEST})
    add_executable(utgfg ${GemmFamilyGeneratorSrc} test/test_gemm_family_generator.cc)
    add_executable(utgg ${GemmGeneratorSrc} test/test_gemm_generator.cc)
    add_executable(utvardecl ${ASTSrc} src/scheme_info.cc test/test_decl_vars.cc)
    add_executable(utloopgen ${ASTSrc} test/test_loop_codegen.cc)
    add_executable(utbuf ${ASTSrc} test/test_buf.cc)
    add_executable(utkercall ${ASTSrc} test/test_kernel_call.cc)
    add_executable(utpackpos ${GemmGeneratorSrc} test/test_find_packpos.cc)
    add_executable(utloopindex ${GemmGeneratorSrc} test/test_loopindex.cc)
    add_executable(utternary ${ASTSrc} test/test_ternary.cc)
    add_executable(utvcopy ${ASTSrc} test/test_vcopy.cc)
    add_executable(utdiffpack test/test_diff_packing.cc)
    add_executable(utpstmt ${ASTSrc} test/test_parallel.cc)
    add_executable(utparapass ${ParallelSrc} test/test_parallel_pass.cc)
    add_executable(utdeepcopy ${ParallelSrc} test/test_deepcopy.cc)
    add_executable(utwalk ${ParallelSrc} test/test_walkast.cc)
endif()

add_executable(genall ${GemmFamilyGeneratorSrc} exes/generate_all.cc)
add_executable(gengemm ${ParallelSrc} exes/gengemm.cc)

if (${BUILD_ARCH} STREQUAL sw6a OR ${BUILD_ARCH} STREQUAL sw8a)

    if (${BUILD_ARCH} STREQUAL sw6a)
        set(SW_ARCH -DLM_SW6A)
    elseif (${BUILD_ARCH} STREQUAL sw8a)
        set(SW_ARCH -DLM_SW8A)
    endif()
    set(KernelSrcDir runtime/sw6a)
    file(GLOB_RECURSE KernelSrc ${KernelSrcDir}/*.S)
    message(STATUS "Build with ${KernelSrc}")

    add_library(dgemmkernel OBJECT ${KernelSrc})
    target_compile_definitions(dgemmkernel PRIVATE DOUBLE)
    set(GemmKernel dgemmkernel)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp ${SW_ARCH}")

    message(STATUS "CXX FLAGS: ${CMAKE_CXX_FLAGS}")
    if (${ENABLE_TEST})
        add_executable(utstgemm $<TARGET_OBJECTS:dgemmkernel>
                                        test/test_gencode.cc)
        target_link_libraries(utstgemm PRIVATE ${BLAS_LIB})
        target_include_directories(utstgemm PRIVATE gencode runtime)

        add_executable(utpack $<TARGET_OBJECTS:dgemmkernel>
                                    test/test_timing_packing.cc)
        target_link_libraries(utpack PRIVATE ${BLAS_LIB})
        target_include_directories(utpack PRIVATE gencode runtime)

        add_executable(utmtgemm $<TARGET_OBJECTS:dgemmkernel>
                                    test/test_parallel_gemm.cc)
        target_link_libraries(utmtgemm PRIVATE ${BLAS_LIB})
        target_include_directories(utmtgemm PRIVATE gencode runtime)

        add_executable(utredcgemm $<TARGET_OBJECTS:dgemmkernel>
                                    test/test_reduction_gemm.cc)
        target_link_libraries(utredcgemm PRIVATE ${BLAS_LIB})
        target_include_directories(utredcgemm PRIVATE gencode runtime)
    endif()

    add_executable(benchgen $<TARGET_OBJECTS:dgemmkernel> bench/bench_gengemm.cc)
    target_link_libraries(benchgen PRIVATE ${BLAS_LIB})
    target_include_directories(benchgen PRIVATE gencode runtime)

    add_executable(benchgenmt $<TARGET_OBJECTS:dgemmkernel> bench/bench_gengemm_mt.cc)
    target_link_libraries(benchgenmt PRIVATE ${BLAS_LIB})
    target_include_directories(benchgenmt PRIVATE gencode runtime)

elseif(${BUILD_ARCH} STREQUAL skylakex)

    set(KernelSrcDir runtime/skylakex)
    file(GLOB_RECURSE GemmKernelSrc ${KernelSrcDir}/dgemm_kernel_16x2_skylakex.c)
    message(STATUS "Build with ${GemmKernelSrc}")

    add_library(dgemm_kernel_pa_pb_c OBJECT ${GemmKernelSrc})
    target_compile_definitions(dgemm_kernel_pa_pb_c PRIVATE PACK_A PACK_B)

    add_library(dgemm_kernel_pa_pb_pc OBJECT ${GemmKernelSrc})
    target_compile_definitions(dgemm_kernel_pa_pb_pc PRIVATE PACK_A PACK_B PACK_C)

    add_library(dgemm_kernel_a_pb_c OBJECT ${GemmKernelSrc})
    target_compile_definitions(dgemm_kernel_a_pb_c PRIVATE PACK_B)
    
    add_library(dgemm_kernel_a_pb_pc OBJECT ${GemmKernelSrc})
    target_compile_definitions(dgemm_kernel_a_pb_pc PRIVATE PACK_B PACK_C)

    add_library(dgemm_kernel_pa_b_c OBJECT ${GemmKernelSrc})
    target_compile_definitions(dgemm_kernel_pa_b_c PRIVATE PACK_A)

    add_library(dgemm_kernel_pa_b_pc OBJECT ${GemmKernelSrc})
    target_compile_definitions(dgemm_kernel_pa_b_pc PRIVATE PACK_A PACK_C)

    add_library(dgemm_kernel_a_b_pc OBJECT ${GemmKernelSrc})
    target_compile_definitions(dgemm_kernel_a_b_pc PRIVATE PACK_C)
    
    add_library(dgemm_kernel_a_b_c OBJECT ${GemmKernelSrc})

    set(GemmKernel dgemm_kernel_pa_pb_c dgemm_kernel_pa_pb_pc dgemm_kernel_a_pb_c dgemm_kernel_a_pb_pc
                    dgemm_kernel_pa_b_c dgemm_kernel_pa_b_pc dgemm_kernel_a_b_c dgemm_kernel_a_b_pc)
    
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -DLM_SKX -mavx512f")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp -DLM_SKX -mavx512f")

    if (${ENABLE_TEST})
        add_executable(utstgemm test/test_gencode.cc)
        target_link_libraries(utstgemm PRIVATE ${BLAS_LIB} ${GemmKernel})
        target_include_directories(utstgemm PRIVATE gencode runtime)

        add_executable(utpack test/test_timing_packing.cc)
        target_link_libraries(utpack PRIVATE ${BLAS_LIB} ${GemmKernel})
        target_include_directories(utpack PRIVATE gencode runtime)

        add_executable(utmtgemm test/test_parallel_gemm.cc)
        target_link_libraries(utmtgemm PRIVATE ${BLAS_LIB} ${GemmKernel})
        target_include_directories(utmtgemm PRIVATE gencode runtime)

        add_executable(utredcgemm test/test_reduction_gemm.cc)
        target_link_libraries(utredcgemm PRIVATE ${BLAS_LIB} ${GemmKernel})
        target_include_directories(utredcgemm PRIVATE gencode runtime)
    endif()

    add_executable(benchgen bench/bench_gengemm.cc)
    target_link_libraries(benchgen PRIVATE ${BLAS_LIB} ${GemmKernel})
    target_include_directories(benchgen PRIVATE gencode runtime)

    add_executable(benchgenmt bench/bench_gengemm_mt.cc)
    target_link_libraries(benchgenmt PRIVATE ${BLAS_LIB} ${GemmKernel})
    target_include_directories(benchgenmt PRIVATE gencode runtime)
endif()

add_executable(bench_bandwidth bench/bench_bandwidth.cc)
target_include_directories(bench_bandwidth PRIVATE runtime test include)

add_executable(bench_peak_flops bench/bench_peak_flops.cc)
target_include_directories(bench_peak_flops PRIVATE test runtime)

add_executable(benchblas bench/bench_blaslib.cc)
target_link_libraries(benchblas PRIVATE ${BLAS_LIB})
