get_filename_component(PARENT_DIR "${CMAKE_CURRENT_LIST_DIR}" DIRECTORY)

project(zf4)

find_package(glfw3 CONFIG REQUIRED)
find_package(OpenAL CONFIG REQUIRED)

add_library(zf4 STATIC
    src/zf4_game.cpp
    src/zf4_window_state.cpp
    src/zf4_window_logic.cpp
    src/zf4_assets_state.cpp
    src/zf4_assets_logic.cpp
    src/zf4_shader_progs.cpp
    src/zf4_renderer.cpp
    src/zf4_sprites_state.cpp
    src/zf4_sprites_logic.cpp
    src/zf4_audio.cpp
    src/zf4_rand.cpp
    src/zf4_component_types.cpp
    src/zf4_scene_types.cpp
    src/zf4_ents.cpp
    src/zf4_scenes.cpp
    ${PARENT_DIR}/vendor/glad/src/glad.c

    include/zf4.h
    include/zf4_game.h
    include/zf4_window.h
    include/zf4_assets.h
    include/zf4_shader_progs.h
    include/zf4_renderer.h
    include/zf4_sprites.h
    include/zf4_audio.h
    include/zf4_rand.h
    include/zf4_scenes.h
    ${PARENT_DIR}/vendor/glad/include/glad/glad.h
    ${PARENT_DIR}/vendor/glad/include/KHR/khrplatform.h
)

target_include_directories(zf4 PRIVATE
	include
	${PARENT_DIR}/zf4_common/include
	${PARENT_DIR}/vendor/glad/include
)

target_link_libraries(zf4 PRIVATE zf4_common glfw OpenAL::OpenAL)

target_compile_definitions(zf4 PRIVATE GLFW_INCLUDE_NONE)

target_compile_options(zf4 PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic>
)
