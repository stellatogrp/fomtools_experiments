# Builds bench_cpg.c against one generated CVXPYgen directory, unchanged:
#   cmake -S bench_c -B <build> -DGEN_DIR=<out/cpg_nN_arm> -DPREFIX=<prefix> -DMODE=OFF|ON|FOM
# The generated c/CMakeLists.txt exports <prefix>_cpg_include / _cpg_src (and
# compiles at -O3); the driver is compiled from those same lists at -O3.
cmake_minimum_required(VERSION 3.5)
project(bench_cpg C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
add_subdirectory(${GEN_DIR}/c generated)

include_directories(${${PREFIX}_cpg_include})
add_executable(bench_cpg ${CMAKE_CURRENT_SOURCE_DIR}/bench_cpg.c ${${PREFIX}_cpg_src})
target_compile_definitions(bench_cpg PRIVATE PREFIX=${PREFIX} MODE_${MODE})
if(DEFINED BENCH_RHO)  # not OSQP_RHO: OSQP's headers define that as its default rho
  target_compile_definitions(bench_cpg PRIVATE BENCH_RHO=${BENCH_RHO})
endif()
target_link_libraries(bench_cpg m)
