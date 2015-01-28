#version 140

in vec2 pos2D; 
uniform mat4 projection;
uniform vec2 entity_position;
uniform vec2 entity_size;

void main() 
{ 
	gl_Position = projection * vec4(entity_size * pos2D + entity_position, 0, 1 ); 

}
