#version 330

// Object normal and position in Object space
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

// Normal in Object space
smooth out vec3 norm;
smooth out vec3 diffuse;
smooth out vec3 uvw;

uniform mat4 projection;
uniform mat4 view;

// Position and size of the Object
uniform vec3 entity_position;
uniform mat3 entity_rotation;
uniform vec3 entity_size;
uniform vec3 entity_color;

void main()
{
	norm = entity_rotation * normal;
	diffuse = entity_color;
	vec3 pos = entity_size * position;
	pos = entity_rotation * pos + entity_position;
	uvw = pos.xyz;
	gl_Position = projection * view * vec4(pos, 1);

}
