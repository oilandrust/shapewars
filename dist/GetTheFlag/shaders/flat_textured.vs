#version 140

in vec3 pos;
in vec3	 tc;

smooth out vec2 texture_coord;
 
uniform mat4 projection;
uniform mat4 view;

uniform vec3 entity_position;
uniform vec3 entity_size;

void main() 
{
	gl_Position = projection * view * vec4(entity_size * pos + entity_position, 1); 
	texture_coord = vec2(0.5f*tc.x, 1.0f-0.5f*tc.y);
}
