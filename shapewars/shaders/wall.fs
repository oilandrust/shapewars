#version 140

in vec3 norm;
in vec3 diffuse;
in vec3 uvw;

out vec4 fragment;

const vec3 lightDir = normalize(vec3(0.5,-0.7,1));
const vec3 cline = vec3(1,0,0);

void main()
{
	// Grid texture
	float u;
	float ur = modf(uvw.x+0.5, u);
	float v;
	float vr = modf(uvw.y+0.5, v);
	float w;
	float wr = modf(uvw.z+0.5, w);

	ur = ur - 0.5;
	vr = vr - 0.5;
	wr = wr - 0.5;
	float ud = exp(-ur*ur/0.0005);
	float vd = exp(-vr*vr/0.0005);
	float wd = exp(-wr*wr/0.0005);

	ud *= 1.0 - abs(dot(norm,vec3(1,0,0)));
	vd *= 1.0 - abs(dot(norm,vec3(0,1,0)));
	wd *= 1.0 - abs(dot(norm,vec3(0,0,1)));

	// Diffuse Lighting
	vec3 diff = mix(diffuse, cline, ud);
	diff = mix(diff, cline, vd);
	diff = mix(diff, cline, wd);

	float nDotL = max(0.f,dot(norm,lightDir));
	vec3 color = nDotL * diff + 0.2 * diff;

	fragment = vec4(color, 1);
}
