#version 330 core
uniform sampler2D gPositionDepth;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];
uniform mat4 projection;

in vec2 frag_tex_coord;
out float frag_color;

void main()
{
	vec3 fragPos = texture(gPositionDepth, frag_tex_coord).xyz;
	vec3 normal = texture(gNormal, frag_tex_coord).xyz;
	vec3 randomVec = texture(texNoise, frag_tex_coord).xyz;

	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);

	mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;
	for (int i = 0; i < 64; ++i)
	{
		vec3 sample = TBN * samples[i];
		vec3 samplePos = fragPos + sample * 0.1;
		vec4 offset = projection * vec4(samplePos, 1.0);
		offset.xy /= offset.w;
		offset.xy = offset.xy * 0.5 + 0.5;

		float sampleDepth = texture(gPositionDepth, offset.xy).w;
		float rangeCheck = smoothstep(0.0, 1.0, length(samplePos - fragPos));
		float depthCheck = (sampleDepth >= samplePos.z + 0.05) ? 1.0 : 0.0;
		occlusion += (rangeCheck * depthCheck);
	}
	occlusion = 1.0 - (occlusion / 64.0);

	frag_color = occlusion;
}