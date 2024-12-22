get_filename_component(PARENT_DIR "${CMAKE_CURRENT_LIST_DIR}" DIRECTORY)

project(zf4_asset_packer)

find_package(cJSON CONFIG REQUIRED)
find_package(Freetype REQUIRED)
find_package(SndFile CONFIG REQUIRED)

add_executable(zf4_asset_packer
	src/zf4ap_main.c
	${PARENT_DIR}/vendor/stb_image/src/stb_image.c

	src/zf4ap.h
)

target_include_directories(zf4_asset_packer PRIVATE
	${PARENT_DIR}/zf4_common/include
	${PARENT_DIR}/vendor/stb_image/include
)

target_link_libraries(zf4_asset_packer PRIVATE zf4_common cjson Freetype::Freetype SndFile::sndfile)
