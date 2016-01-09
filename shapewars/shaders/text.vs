#version 330

// Object normal and position in Object space
layout(location = 0) in vec2 position;
layout(location = 3) in vec2 uv;

// Normal in Object space
smooth out vec2 tex_coords;

uniform mat4 projection;

void main()
{
	tex_coords = uv;
	vec2 pos = 2*position / vec2(640, 480);
	pos.y = 1-pos.y;
	gl_Position = vec4(pos, 1,1);
}
