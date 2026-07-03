cmake_minimum_required(VERSION 3.16)
project(FHE_Gravity VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options(-O3 -march=native)

add_library(fhe_core INTERFACE)
target_include_directories(fhe_core INTERFACE ${CMAKE_SOURCE_DIR})

# Glob all cpp files for the demo executable
file(GLOB DEMO_SOURCES *.cpp)
add_executable(fhe_grav_demo ${DEMO_SOURCES})
target_link_libraries(fhe_grav_demo PRIVATE fhe_core)
