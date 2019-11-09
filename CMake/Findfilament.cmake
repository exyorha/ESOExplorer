find_path(
	filament_ROOT
	NAMES include/filament/Engine.h
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(filament
  FOUND_VAR filament_FOUND
  REQUIRED_VARS
	filament_ROOT
)

if(filament_FOUND)
	add_library(filament::filament INTERFACE IMPORTED)
	set_property(TARGET filament::filament PROPERTY INTERFACE_LINK_LIBRARIES 
		"${filament_ROOT}/lib/x86_64/md$<$<CONFIG:Debug>:d>/backend.lib"
		"${filament_ROOT}/lib/x86_64/md$<$<CONFIG:Debug>:d>/bluegl.lib"
		"${filament_ROOT}/lib/x86_64/md$<$<CONFIG:Debug>:d>/filabridge.lib"
		"${filament_ROOT}/lib/x86_64/md$<$<CONFIG:Debug>:d>/filaflat.lib"
		"${filament_ROOT}/lib/x86_64/md$<$<CONFIG:Debug>:d>/filament.lib"
		"${filament_ROOT}/lib/x86_64/md$<$<CONFIG:Debug>:d>/geometry.lib"
		"${filament_ROOT}/lib/x86_64/md$<$<CONFIG:Debug>:d>/ibl.lib"
		"${filament_ROOT}/lib/x86_64/md$<$<CONFIG:Debug>:d>/matdbg.lib"
		"${filament_ROOT}/lib/x86_64/md$<$<CONFIG:Debug>:d>/utils.lib"
		opengl32
	)
	set_property(TARGET filament::filament PROPERTY INTERFACE_INCLUDE_DIRECTORIES
		"${filament_ROOT}/include"
	)

	add_executable(filament::matc IMPORTED)
	set_property(TARGET filament::matc PROPERTY IMPORTED_LOCATION "${filament_ROOT}/bin/matc.exe")
		
endif()
