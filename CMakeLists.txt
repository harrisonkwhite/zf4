cmake_minimum_required(VERSION 3.28)

project(zf4)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W2 /WX")
endif()

get_filename_component(PARENT_DIR "${CMAKE_CURRENT_LIST_DIR}" DIRECTORY)
find_package(glfw3 CONFIG REQUIRED)

add_library(zf4 STATIC
    src/zf4_game.cpp
    src/zf4_graphics.cpp
    src/zf4_audio.cpp
    src/zf4_pathfinding.cpp
    src/zf4_rand.cpp
    src/zf4_mem.cpp
    src/zf4_io.cpp
    src/zf4_math.cpp
    src/external/glad.c
    src/external/stb_image.c
	src/external/stb_truetype.c
	src/external/miniaudio.c

    include/zf4.h
    include/zf4_game.h
    include/zf4_graphics.h
    include/zf4_audio.h
    include/zf4_pathfinding.h
    include/zf4_rand.h
    include/zf4_mem.h
    include/zf4_io.h
    include/zf4_math.h
)

target_include_directories(zf4 PRIVATE
	include
    include/external
)

target_link_libraries(zf4 PRIVATE glfw)

target_compile_definitions(zf4 PRIVATE GLFW_INCLUDE_NONE)
