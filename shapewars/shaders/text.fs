#version 140

in vec2 tex_coords;
in vec3 col;

out vec4 fragment;

uniform sampler2D diffuse;

void main()
{
	float alpha = texture2D(diffuse, vec2(tex_coords.x, tex_coords.y)).r;
	fragment = vec4(col, alpha);
}
