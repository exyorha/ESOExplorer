add_library(granny INTERFACE)

add_custom_target(granny-import-library
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/granny.lib
)

target_include_directories(granny INTERFACE include)
target_link_libraries(granny INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/granny.lib)
add_dependencies(granny granny-import-library)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/granny.lib
	COMMAND
		lib
		${CMAKE_STATIC_LINKER_FLAGS}
		/def:${CMAKE_CURRENT_SOURCE_DIR}/granny.def
		/out:${CMAKE_CURRENT_BINARY_DIR}/granny.lib
	MAIN_DEPENDENCY granny.def
	VERBATIM
)

set(GRANNY2_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/granny2_x64.dll PARENT_SCOPE)