#version 140

in vec3 norm;
in vec3 diffuse;
in vec2 uv;

out vec4 fragment;

const vec3 lightDir = normalize(vec3(0.5,-0.7,1));
const vec3 cline = vec3(0.6);

void main()
{
	// Grid texture
	float u;
	float ur = modf(0.25*uv.x+0.5, u);
	float v;
	float vr = modf(0.25*uv.y+0.5, v);

	ur = ur - 0.5;
	vr = vr - 0.5;
	float ud = exp(-ur*ur/0.0001);
	float vd = exp(-vr*vr/0.0001);

	// Diffuse Lighting
	vec3 diff = mix(diffuse, cline, vd);
	diff = mix(diff, cline, ud);

	float nDotL = max(0.f,dot(norm,lightDir));
	vec3 color = nDotL * diff + 0.2 * diff;

	fragment = vec4(color, 1);
}
