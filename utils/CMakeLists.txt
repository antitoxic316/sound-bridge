add_compile_options(${C_LIBS} ${C_FLAGS})

add_library(utils SHARED debug.c args.c)

target_include_directories(utils PUBLIC CMAKE_CURRENT_SOURCE_DIR)
target_include_directories(utils PRIVATE ${CMAKE_SOURCE_DIR}/alsa_backend)