cmake_minimum_required(VERSION 3.28.1)
project(sound_bridge C)

enable_testing()

set(DEBUG TRUE)

set(C_LIBS "-lasound")
set(C_FLAGS "-Wall" "-Werror")
if(DEBUG)
  set(C_FLAGS ${C_FLAGS} "-g")
endif()

add_compile_options(${C_LIBS} ${C_FLAGS})

add_subdirectory(alsa_backend)
add_subdirectory(utils)
add_subdirectory(core)
add_subdirectory(client)
add_subdirectory(server)

add_subdirectory(test)