project(ShapeWars)
add_executable(ShapeWars
	src/ShapeWars.cpp
	src/ShapeWars.h
	src/Input.h
	src/Input.cpp
	src/Level.h
	src/Level.cpp
	src/Vec2.h
	src/Vec3.h
	src/Mat4.cpp
	src/Mat4.h
	src/Mat3.cpp
	src/Mat3.h
	src/OpenGLClient.h
	src/OpenGLClient.cpp
	src/Entities.cpp
	src/Entities.h
	src/Animation.cpp
	src/Animation.h
	src/Renderer.cpp
	src/Renderer.h
	src/Mesh.cpp
	src/Mesh.h
  	src/NavMeshGen.cpp
	src/NavMeshGen.h
	src/NavMesh.cpp
	src/NavMesh.h
	src/NavMeshQuery.cpp
	src/NavMeshQuery.h
	src/Debug.cpp
	src/Debug.h
	src/Game.cpp
	src/Game.h

	src/stb_image.cpp
)
target_link_libraries(ShapeWars ${SDL2_LIBRARY})
target_link_libraries(ShapeWars ${GLEW_LIBRARY})
target_link_libraries(ShapeWars ${OPENGL_LIBRARIES})

install(TARGETS ShapeWars RUNTIME DESTINATION ${BIN_DIR})
