set(commands)
set(generated_materials)

foreach(dir_file IN ITEMS
	Filesystem.dir
	SupportedVersions.dir
)

	list(APPEND commands COMMAND cmake -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/${dir_file} $<TARGET_FILE_DIR:ESOBrowser>)
	
endforeach()

foreach(mat_file IN ITEMS
	ESOLikeMaterialMasked
	ESOLikeMaterialOpaque
	ESOLikeMaterialTransparent
)

	list(APPEND generated_files ${CMAKE_CURRENT_BINARY_DIR}/${mat_file}.matc)

	add_custom_command(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${mat_file}.matc
		COMMAND filament::matc -o ${CMAKE_CURRENT_BINARY_DIR}/${mat_file}.matc ${CMAKE_CURRENT_SOURCE_DIR}/${mat_file}.mat
		MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${mat_file}.mat
		VERBATIM
	)
	
	list(APPEND commands COMMAND cmake -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/${mat_file}.matc $<TARGET_FILE_DIR:ESOBrowser>)
endforeach()

add_custom_target(shaders ALL DEPENDS ${generated_files})
set(AssetCopyCommands ${commands} PARENT_SCOPE)
