project(GetTheFlag)
add_executable(GetTheFlag 
	src/GetTheFlag.cpp
	src/Input.h
	src/Input.cpp
)
target_link_libraries(GetTheFlag ${SDL2_LIBRARY})
target_link_libraries(GetTheFlag ${SDL2IMAGE_LIBRARY})
install(TARGETS GetTheFlag RUNTIME DESTINATION ${BIN_DIR})
