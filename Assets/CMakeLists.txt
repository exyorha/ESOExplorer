set(commands)

foreach(dir_file IN ITEMS
	Filesystem.dir
	SupportedVersions.dir
)

	list(APPEND commands COMMAND cmake -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/${dir_file} $<TARGET_FILE_DIR:ESOBrowser>)

endforeach()

set(AssetCopyCommands ${commands} PARENT_SCOPE)
