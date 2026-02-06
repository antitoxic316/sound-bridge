cmake_minimum_required(VERSION 3.31.5)
project(sound_bridge C)

set(C_LIBS -lasound)
set(C_FLAGS -Wall -g)

add_compile_options(${C_LIBS} ${C_FLAGS})

add_subdirectory(alsa_backend)
add_subdirectory(utils)
add_subdirectory(client)
add_subdirectory(server)
