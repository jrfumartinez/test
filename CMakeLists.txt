cmake_minimum_required(VERSION 3.10.0)
project(test VERSION 0.1.0 LANGUAGES C CXX)


set(CMAKE_LIBRARY_PATH /lib64/)
set(HEADERS_DIR /include/)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lnetcdf")
find_library(NETCDF_LIB netcdf)

add_executable(test alglibinternal.cpp alglibmisc.cpp ap.cpp dataanalysis.cpp diffequations.cpp fasttransforms.cpp integration.cpp interpolation.cpp kernels_avx2.cpp kernels_fma.cpp kernels_neon.cpp kernels_rvv10.cpp kernels_sse2.cpp linalg.cpp main.cpp minlp.cpp optimization.cpp solvers.cpp specialfunctions.cpp statistics.cpp)
target_link_libraries(test ${NETCDF_LIB})
target_include_directories(test PRIVATE ${HEADERS_DIR})

