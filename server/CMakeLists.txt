set(LIB_SRC
				"src/server.c"
)



add_executable(server ${LIB_SRC})

target_include_directories(server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)


