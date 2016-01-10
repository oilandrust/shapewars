#version 140

in vec2 tex_coords;

out vec4 fragment;

uniform sampler2D diffuse;

void main()
{
	vec4 col = texture(diffuse, vec2(tex_coords.x, tex_coords.y));
	fragment = vec4(1,1,1,col.r);
}
