cmake_minimum_required(VERSION 3.14)

project(ESOBrowser)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(filament REQUIRED)

add_subdirectory(ESOData)
add_subdirectory(DirectXTex)
add_subdirectory(granny2)
add_subdirectory(Assets)
add_subdirectory(ESOBrowser)
