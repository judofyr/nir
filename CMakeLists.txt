cmake_minimum_required(VERSION 3.2)
project(Nir)
set(CMAKE_CXX_STANDARD 14)
file(GLOB SOURCES "src/*.cc")

add_library(nir ${SOURCES})

add_executable(demo demo.cc)
target_link_libraries(demo nir)

