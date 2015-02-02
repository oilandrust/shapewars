#version 140

in vec3 norm;
in vec3 diffuse;

out vec4 fragment; 

const vec3 red = vec3(1,0,0);
const vec3 lightDir = normalize(vec3(0.5,-0.7,1));

void main() 
{
	// Diffuse Lighting
	float nDotL = max(0.f,dot(norm,lightDir));
	vec3 color = nDotL * diffuse + 0.2 * diffuse;

	fragment = vec4(color, 1); 
}
