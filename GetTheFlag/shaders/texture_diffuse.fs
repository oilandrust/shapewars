#version 140

in vec3 norm;
in vec2 tex_coords;

out vec4 fragment; 

uniform sampler2D diffuse;

const vec3 lightDir = normalize(vec3(0.5,-0.7,1));

void main() 
{
	// Diffuse Lighting
	vec4 diff = texture(diffuse, vec2(tex_coords.x,1-tex_coords.y)).gbar;
	float nDotL = max(0.f,dot(norm,lightDir));
	vec4 color = nDotL * diff + 0.4 * diff;

	fragment = vec4(color.rgb, 1); 
}
