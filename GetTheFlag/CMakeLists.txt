project(GetTheFlag)
add_executable(GetTheFlag 
	src/GetTheFlag.cpp	
	src/GetTheFlag.h
	src/Input.h
	src/Input.cpp
	src/Level.h
	src/Level.cpp
	src/Vec2.h
	src/Vec3.h
	src/Mat4.h
	src/Mat3.h
	src/OpenGLClient.h
	src/OpenGLClient.cpp
	src/Entities.cpp
	src/Entities.h
	src/Renderer.cpp
	src/Renderer.h
	src/Mesh.cpp
	src/Mesh.h
)
target_link_libraries(GetTheFlag ${SDL2_LIBRARY})
target_link_libraries(GetTheFlag ${GLEW_LIBRARY})
target_link_libraries(GetTheFlag ${OPENGL_LIBRARIES})

install(TARGETS GetTheFlag RUNTIME DESTINATION ${BIN_DIR})
