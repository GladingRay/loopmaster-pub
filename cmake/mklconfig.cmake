set(BUILD_ARCH skylakex) # options: [sw6a/sw8a/skylakex]
set(ENABLE_ASAN OFF) # options: [ON/OFF]
set(ENABLE_TEST ON) # options: [ON/OFF]
set(CMAKE_VERBOSE_MAKEFILE OFF) # options: [ON/OFF]
set(MKL_DIR /home/guorui/oneapi/mkl/latest/lib/cmake/mkl)
find_package(MKL REQUIRED CONFIG)
link_directories(/home/guorui/oneapi/mkl/latest/lib/)
set(BLAS_LIB libmkl_intel_lp64.a libmkl_gnu_thread.a libmkl_core.a m dl)
# set(BLAS_LIB MKL::MKL)