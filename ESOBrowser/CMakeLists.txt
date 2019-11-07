add_executable(ESOBrowser WIN32
	BinaryFileViewWidget.cpp
	BinaryFileViewWidget.h
	DataStorage.cpp
	DataStorage.h
	DDSFileViewWidget.cpp
	DDSFileViewWidget.h
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
	HexView.cpp
	HexView.h
	main.cpp
	PersistentTabWidget.cpp
	PersistentTabWidget.h
	SupportedVersionsDirectiveFile.cpp
	SupportedVersionsDirectiveFile.h
	TextFileViewWidget.cpp
	TextFileViewWidget.h
)

target_link_libraries(ESOBrowser PRIVATE Qt5::Widgets ESOData DirectXTex)

set_target_properties(ESOBrowser PROPERTIES 
	AUTOMOC TRUE
	AUTOUIC TRUE
	AUTORCC TRUE
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED TRUE
)

add_custom_command(TARGET ESOBrowser POST_BUILD ${AssetCopyCommands})
