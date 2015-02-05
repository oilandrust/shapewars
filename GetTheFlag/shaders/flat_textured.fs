#version 140

smooth in vec2 texture_coord;
out vec4 fragment; 

uniform sampler2D sprite_tex;

void main() 
{
	fragment = texture(sprite_tex, texture_coord).barg;
}
