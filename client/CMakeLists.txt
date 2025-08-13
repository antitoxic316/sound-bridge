set(LIB_SRC
				"src/client.c"
)

add_executable(client ${LIB_SRC})

target_include_directories(client PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

