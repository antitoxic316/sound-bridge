add_compile_options(${C_LIBS} ${C_FLAGS})

add_library(utils SHARED debug.c)

target_include_directories(utils PUBLIC CMAKE_CURRENT_SOURCE_DIR)
