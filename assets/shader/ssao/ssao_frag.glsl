#version 330 core
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];
uniform mat4 projection;

in vec2 frag_tex_coord;
out float frag_color;

const vec2 noiseScale = vec2(800.0f / 4.0f, 600.0f / 4.0f);

void main()
{
	vec3 fragPos = texture(gPosition, frag_tex_coord).xyz;
	vec3 normal = normalize(texture(gNormal, frag_tex_coord).xyz);
	vec3 randomVec = normalize(texture(texNoise, frag_tex_coord * noiseScale).xyz);

	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);

	mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;
	for (int i = 0; i < 64; ++i)
	{
		vec3 sample = TBN * samples[i];
		vec3 samplePos = fragPos + sample * 1.0;
		vec4 offset = projection * vec4(samplePos, 1.0);
		offset.xy /= offset.w;
		offset.xy = offset.xy * 0.5 + 0.5;

		float sampleDepth = texture(gPosition, offset.xy).z;
		float rangeCheck = smoothstep(0.0, 1.0, 1.0 / abs(fragPos.z - sampleDepth));
		occlusion += (sampleDepth >= samplePos.z ? 1.0 : 0.0) * rangeCheck;
	}
	occlusion = 1.0 - (occlusion / 64.0);

	frag_color = occlusion;
}