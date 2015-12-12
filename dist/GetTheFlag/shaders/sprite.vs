#version 140

in vec2 pos2D;
in vec2 tc;

smooth out vec2 texture_coord;
 
uniform mat4 projection;
uniform mat4 view;

uniform vec2 entity_position;
uniform vec2 entity_size;
uniform mat2 entity_rotation;

void main() 
{
	vec2 rotPos = entity_rotation * pos2D; 
	gl_Position = projection * view * vec4(entity_size * rotPos + entity_position, 0.f, 1 ); 
	texture_coord = vec2(tc.x, 1.0f-tc.y);
}
