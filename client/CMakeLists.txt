set(LIB_SRC
				"src/client.c"
)

add_executable(sb-client ${LIB_SRC})

target_include_directories(sb-client PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(sb-client PRIVATE ${CMAKE_SOURCE_DIR}/alsa_backend)
target_include_directories(sb-client PRIVATE ${CMAKE_SOURCE_DIR}/utils)

target_link_libraries(sb-client PRIVATE alsa_backend)
target_link_libraries(sb-client PRIVATE utils)