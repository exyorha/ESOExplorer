cmake_minimum_required(VERSION 3.14)

project(ESOBrowser)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_subdirectory(ESOData)
add_subdirectory(DirectXTex)
add_subdirectory(Assets)
add_subdirectory(ESOBrowser)
