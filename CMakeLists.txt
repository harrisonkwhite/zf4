cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(zf4)

add_subdirectory(zf4)
add_subdirectory(zf4_asset_packer)
add_subdirectory(zf4_common)
