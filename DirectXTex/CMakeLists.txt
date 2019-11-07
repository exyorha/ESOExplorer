add_library(DirectXTex STATIC EXCLUDE_FROM_ALL
	DirectXTex/DirectXTex/DirectXTex.h
	DirectXTex/DirectXTex/DirectXTexp.h
	DirectXTex/DirectXTex/BC.h
	DirectXTex/DirectXTex/DDS.h
	DirectXTex/DirectXTex/scoped.h
	DirectXTex/DirectXTex/BC.cpp
	DirectXTex/DirectXTex/BC4BC5.cpp
	DirectXTex/DirectXTex/BC6HBC7.cpp
	DirectXTex/DirectXTex/DirectXTexConvert.cpp
	DirectXTex/DirectXTex/DirectXTexCompress.cpp
	DirectXTex/DirectXTex/DirectXTexDDS.cpp
	DirectXTex/DirectXTex/DirectXTexImage.cpp
	DirectXTex/DirectXTex/DirectXTexMipmaps.cpp
	DirectXTex/DirectXTex/DirectXTexTGA.cpp
	DirectXTex/DirectXTex/DirectXTexUtil.cpp
	DirectXTex/DirectXTex/DirectXTexWIC.cpp)

target_include_directories(DirectXTex PUBLIC "${CMAKE_CURRENT_LIST_DIR}/DirectXTex/DirectXTex")

target_link_libraries(DirectXTex
	shlwapi.lib
	windowscodecs.lib)