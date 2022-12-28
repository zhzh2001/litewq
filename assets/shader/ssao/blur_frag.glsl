#version 330 core
in vec2 frag_tex_coord;
out float frag_color;

uniform sampler2D ssaoInput;

void main()
{
	vec2 texelSize = 1.0 / vec2(textureSize(ssaoInput, 0));
	float result = 0.0;
	for (int x = -2; x < 2; ++x)
	{
		for (int y = -2; y < 2; ++y)
		{
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			result += texture(ssaoInput, frag_tex_coord + offset).r;
		}
	}
	result /= 16.0;
	frag_color = result;
}