project(GetTheFlag)
add_executable(GetTheFlag src/GetTheFlag.cpp)
target_link_libraries(GetTheFlag ${SDL2_LIBRARY})
install(TARGETS GetTheFlag RUNTIME DESTINATION ${BIN_DIR})
