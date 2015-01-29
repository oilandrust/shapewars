#version 140

in vec2 pos2D;
in vec2 tc;

smooth out vec2 texture_coord;
 
uniform mat4 projection;
uniform vec2 entity_position;
uniform vec2 entity_size;

void main() 
{ 
	gl_Position = projection * vec4(entity_size * pos2D + entity_position, 0, 1 ); 
	texture_coord = vec2(tc.x, 1.0f-tc.y);
}
