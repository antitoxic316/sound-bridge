add_compile_options(${C_LIBS} ${C_FLAGS})

add_library(alsa_backend SHARED sound.c sharedbuffer.c)

target_include_directories(alsa_backend PUBLIC CMAKE_CURRENT_SOURCE_DIR)
target_include_directories(alsa_backend PRIVATE ${CMAKE_SOURCE_DIR}/utils)

find_package(ALSA)
target_link_libraries(alsa_backend PUBLIC ALSA::ALSA)
target_link_libraries(alsa_backend PRIVATE utils)