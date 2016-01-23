#version 330

// Object normal and position in Object space
layout(location = 0) in vec2 position;
layout(location = 2) in vec3 color;
layout(location = 3) in vec2 uv;

// Normal in Object space
smooth out vec2 tex_coords;
smooth out vec3 col;

uniform vec2 resolution;

void main()
{
	tex_coords = uv;
	vec2 pos = 2 * position / resolution;
	pos.x = pos.x-1;
	pos.y = 1-pos.y;
	col = color;
	gl_Position = vec4(pos, 1,1);
}
