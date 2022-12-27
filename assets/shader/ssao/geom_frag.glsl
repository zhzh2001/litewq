#version 330 core
layout (location = 0) out vec4 gPositionDepth;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 frag_tex_coord;
in vec3 frag_pos;
in vec3 frag_normal;

uniform sampler2D tex;

const float NEAR = 0.1f;
const float FAR = 100.0f;
float LinearizeDepth(float depth)
{
	float z = depth * 2.0 - 1.0; // Back to NDC
	return (2.0 * NEAR * FAR) / (FAR + NEAR - z * (FAR - NEAR));
}

void main()
{
	gPositionDepth = vec4(frag_pos, LinearizeDepth(frag_tex_coord.x));
	gNormal = normalize(frag_normal);
	gAlbedoSpec = texture(tex, frag_tex_coord);
}