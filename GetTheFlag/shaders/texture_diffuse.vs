#version 330

// Object normal and position in Object space
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 3) in vec2 uvs;

// Normal in Object space
smooth out vec3 norm;
smooth out vec2 tex_coords;
 
uniform mat4 projection;
uniform mat4 view;

// Position and size of the Object
uniform vec3 entity_position;
uniform mat3 entity_rotation;
uniform vec3 entity_size;


void main() 
{
	norm = normal;
	tex_coords = uvs;
	gl_Position = projection * view * vec4(entity_size * position + entity_position, 1); 

}