set(LIB_SRC
				"src/server.c"
)

add_executable(sb-server ${LIB_SRC})

target_include_directories(sb-server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(sb-server PRIVATE ${CMAKE_SOURCE_DIR}/alsa_backend)
target_include_directories(sb-server PRIVATE ${CMAKE_SOURCE_DIR}/utils)

target_link_libraries(sb-server PRIVATE alsa_backend utils)