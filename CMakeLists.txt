# CMakeLists.txt for Imzadi game engine project.

cmake_minimum_required(VERSION 3.27)

project(Imzadi)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(Engine)
add_subdirectory(Games)
add_subdirectory(Tools)