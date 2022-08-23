cmake_minimum_required(VERSION 3.0.0)
project(patchgame VERSION 0.1.0)

include(CTest)
enable_testing()

aux_source_directory(. DIR_SRCS)
add_executable(patchgame ${DIR_SRCS})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

# set(CMAKE_CXX_FLAGS "-O3 -pg")
set(CMAKE_CXX_FLAGS "-O3 -fopenmp -pg")
include(CPack)
