# cmd args
# CMAKE_BUILD_TYPE: [cmake]
# BUILD_ARCH: [sw6a/x86]

cmake_minimum_required(VERSION 3.5)

project(Tuner VERSION 1.0 LANGUAGES CXX ASM C)

set(LoopMasterRoot ..)

set(CMAKE_VERBOSE_MAKEFILE ON)
include_directories(${LoopMasterRoot}/include)
include_directories(.)
include_directories(${LoopMasterRoot}/runtime)

if (${BUILD_ARCH} STREQUAL sw6a)
    set(BLASLib /home/iscas_22/guorui/releaselibs/libswblas.a)
    set(KernelSrcDir ${LoopMasterRoot}/runtime/sw6a)
    file(GLOB_RECURSE KernelSrc ${KernelSrcDir}/*.S)
    message(STATUS "Build with ${KernelSrc}")

    add_library(dgemmkernel OBJECT ${KernelSrc})
    target_compile_definitions(dgemmkernel PRIVATE DOUBLE)
    set(GemmKernel dgemmkernel)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -DLM_SW6A")

    add_executable(gemm.exe $<TARGET_OBJECTS:dgemmkernel> test_one.cc)
    target_link_libraries(gemm.exe PRIVATE ${BLASLib})
    target_include_directories(gemm.exe PRIVATE ${LoopMasterRoot}/gencode
                                                ${LoopMasterRoot}/test)

    add_library(gengemm STATIC $<TARGET_OBJECTS:dgemmkernel> fortran_interface.cc)
    target_include_directories(gengemm PRIVATE ${LoopMasterRoot}/gencode)

elseif(${BUILD_ARCH} STREQUAL skylakex)
    set(BLASLib /home/guorui/BLASBenchmark/BLASINSTALL/OpenBLAS/lib/libopenblas.a)
    set(KernelSrcDir ${LoopMasterRoot}/runtime/skylakex)
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

    add_executable(gemm.exe test_one.cc)
    target_link_libraries(gemm.exe PRIVATE ${BLASLib} ${GemmKernel})
    target_include_directories(gemm.exe PRIVATE ${LoopMasterRoot}/gencode
                                                ${LoopMasterRoot}/test)

    add_library(gengemm STATIC fortran_interface.cc)
    target_link_libraries(gengemm PRIVATE ${GemmKernel})
    target_include_directories(gengemm PRIVATE ${LoopMasterRoot}/gencode)

endif()

