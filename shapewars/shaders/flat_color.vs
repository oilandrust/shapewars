#version 330

// Object normal and position in Object space
layout(location = 0) in vec3 position;

// Normal in Object space
smooth out vec3 color;

uniform mat4 projection;
uniform mat4 view;

// Position and size of the Object
uniform vec3 entity_position;
uniform mat3 entity_rotation;
uniform vec3 entity_size;
uniform vec3 entity_color;

void main()
{
	color = entity_color;
	vec3 pos = entity_size * position;
	pos = entity_rotation * pos + entity_position;
	gl_Position = projection * view * vec4(pos, 1);

}
