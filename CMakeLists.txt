# CMakeLists.txt for CollisionSystem project.

cmake_minimum_required(VERSION 3.27)

project(CollisionSystem)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(Library)
add_subdirectory(Sandbox)
add_subdirectory(Controller)