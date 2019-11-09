set(commands)

foreach(dir_file IN ITEMS
	Filesystem.dir
	SupportedVersions.dir
)

	list(APPEND commands COMMAND cmake -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/${dir_file} $<TARGET_FILE_DIR:ESOBrowser>)
	
endforeach()

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ESOLikeMaterial.matc
	COMMAND filament::matc -o ${CMAKE_CURRENT_BINARY_DIR}/ESOLikeMaterial.matc ${CMAKE_CURRENT_SOURCE_DIR}/ESOLikeMaterial.mat
	MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/ESOLikeMaterial.mat
	VERBATIM
)

add_custom_target(shaders ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/ESOLikeMaterial.matc)
list(APPEND commands COMMAND cmake -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/ESOLikeMaterial.matc $<TARGET_FILE_DIR:ESOBrowser>)

set(AssetCopyCommands ${commands} PARENT_SCOPE)
