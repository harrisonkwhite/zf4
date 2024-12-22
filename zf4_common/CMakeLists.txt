project(zf4_common)

add_library(zf4_common STATIC
    src/zf4c_io.c
    src/zf4c_math.c
    src/zf4c_mem.c
    
    include/zf4c.h
    include/zf4c_io.h
    include/zf4c_math.h
    include/zf4c_assets.h
    include/zf4c_mem.h
)

target_include_directories(zf4_common PRIVATE include)
