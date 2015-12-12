cmake_minimum_required(VERSION 2.6)
project(fbxtom3d)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${fbxtom3d_SOURCE_DIR}/cmake")
set(BIN_DIR ${fbxtom3d_SOURCE_DIR}/bin)

find_package(FBX REQUIRED)
include_directories(${FBX_INCLUDE_DIR})

add_executable(fbxtom3d 
	fbxtom3d/main.cpp
)

target_link_libraries(fbxtom3d ${FBX_LIBRARY})
install(TARGETS fbxtom3d RUNTIME DESTINATION ${BIN_DIR})
