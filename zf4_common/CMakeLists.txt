project(zf4_common)

add_library(zf4_common STATIC
    src/zf4c_logging.c
    src/zf4c_math.c
    src/zf4c_mem.c
    
    include/zf4c_logging.h
    include/zf4c_math.h
    include/zf4c_mem.h
)

target_include_directories(zf4_common PRIVATE include)
