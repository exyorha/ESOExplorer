add_executable(ESOBrowser
	BinaryFileViewWidget.cpp
	BinaryFileViewWidget.h
	DataStorage.cpp
	DataStorage.h
	DDSFileViewWidget.cpp
	DDSFileViewWidget.h
	DDSTexture.cpp
	DDSTexture.h
	DirectiveFile.cpp
	DirectiveFile.h
	DummyTabWidget.cpp
	DummyTabWidget.h
	DummyTabWidget.ui
	ESOBrowserMainWindow.cpp
	ESOBrowserMainWindow.h
	ESOBrowserMainWindow.ui
	ESOBrowserSelectDepotDialog.cpp
	ESOBrowserSelectDepotDialog.h
	ESOBrowserSelectDepotDialog.ui
	ESOFilesystemModel.cpp
	ESOFilesystemModel.h
	ESOGraphicsTypes.cpp
	ESOGraphicsTypes.h
	FilamentEngineInstance.cpp
	FilamentEngineInstance.h
	FilamentTypeHelpers.cpp
	FilamentTypeHelpers.h
	FilamentViewport.cpp
	FilamentViewport.h
	FilesystemBrowserWidget.cpp
	FilesystemBrowserWidget.h
	FilesystemBrowserWidget.ui
	FilesystemDirectiveFile.cpp
	FilesystemDirectiveFile.h
	FileTypeDetector.cpp
	FileTypeDetector.h
	FileViewWidget.cpp
	FileViewWidget.h
	FileViewWidget.ui
	Granny2FileViewWidget.cpp
	Granny2FileViewWidget.h
	Granny2Model.cpp
	Granny2Model.h
	Granny2Renderable.cpp
	Granny2Renderable.h
	Granny2TypeHelpers.cpp
	Granny2TypeHelpers.h
	HexView.cpp
	HexView.h
	main.cpp
	ModelViewerWidget.cpp
	ModelViewerWidget.h
	ModelViewerWidget.ui
	PersistentTabWidget.cpp
	PersistentTabWidget.h
	SupportedVersionsDirectiveFile.cpp
	SupportedVersionsDirectiveFile.h
	TextFileViewWidget.cpp
	TextFileViewWidget.h
)

target_link_libraries(ESOBrowser PRIVATE Qt5::Widgets ESOData DirectXTex granny filament::filament)

set_target_properties(ESOBrowser PROPERTIES 
	AUTOUIC TRUE
	AUTOMOC TRUE
	AUTORCC TRUE
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED TRUE
)

add_custom_command(TARGET ESOBrowser POST_BUILD
	${AssetCopyCommands}
	COMMAND cmake -E copy_if_different ${GRANNY2_EXECUTABLE} $<TARGET_FILE_DIR:ESOBrowser>
)
add_dependencies(ESOBrowser shaders)
