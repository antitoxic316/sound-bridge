set(LIB_SRC
				"src/statistic-if.c"
)
add_compile_options(${C_LIBS} ${C_FLAGS})

add_library(core SHARED ${LIB_SRC})

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)