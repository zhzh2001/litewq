#version 330 core
out vec4 frag_color;
in vec2 frag_tex_coord;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D ssao;

struct Light
{
    vec3 Position;
    vec3 Color;
    float Linear;
    float Quadratic;
};

uniform Light light;

void main() {
	vec3 frag_pos = texture(gPosition, frag_tex_coord).rgb;
	vec3 frag_normal = texture(gNormal, frag_tex_coord).rgb;
	vec4 frag_albedo = texture(gAlbedo, frag_tex_coord);
	float ao = texture(ssao, frag_tex_coord).r;

	// calculate lighting as usual
    vec3 ambient = vec3(0.3 * frag_albedo * ao);
    vec3 lighting = ambient;
    vec3 viewDir = normalize(-frag_pos);
    // diffuse shading
    vec3 lightDir = normalize(light.Position - frag_pos);
    vec3 diffuse = max(dot(frag_normal, lightDir), 0.0) * frag_albedo.rgb * light.Color;
    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    vec3 specular = pow(max(dot(frag_normal, halfwayDir), 0.0), 32.0) * light.Color;
    // attenuation
    float distance = length(light.Position - frag_pos);
    float attenuation = 1.0 / (1.0 + light.Linear * distance + light.Quadratic * (distance * distance));
    // combine results
    lighting += (diffuse + specular) * attenuation;

    frag_color = vec4(lighting, 1.0);
}