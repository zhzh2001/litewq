#version 330 core
out vec4 frag_color;
in vec2 frag_tex_coord;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D ssao;

struct Material {
    sampler2D Kd;
    vec3 Ks; // specular coeff.
    float highlight_decay; // control the size of highlight.
};

struct PointLight {
    vec3 pos;
    vec3 Ia;
    vec3 Id;
    vec3 Is;
};

uniform vec3 view_pos;
uniform Material material;
uniform PointLight light;

void main() {
	vec3 frag_pos = texture(gPosition, frag_tex_coord).rgb;
	vec3 frag_normal = texture(gNormal, frag_tex_coord).rgb;
	vec3 frag_albedo = texture(gAlbedo, frag_tex_coord).rgb;
	float ao = texture(ssao, frag_tex_coord).r;

	// calculate lighting as usual
    vec4 DiffuseMapTexColor = vec4(frag_albedo, 1.0f);
    // ambient
    vec3 La = DiffuseMapTexColor.rgb * light.Ia * ao;
    // diffuse
    vec3 norm = normalize(frag_normal);
    vec3 light_dir = normalize(light.pos - frag_pos);
    float diff_coef = max(dot(norm, light_dir), 0.0f);
    vec3 Ld = diff_coef * DiffuseMapTexColor.rgb * light.Id;
    // specular
    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 half_vec = normalize(light_dir + view_dir);
    float spec_coef = pow(max(dot(half_vec, norm), 0.0f), material.highlight_decay);
    vec3 Ls = spec_coef * material.Ks * light.Is;

    vec3 L = La + Ld + Ls;
    frag_color = vec4(L, 1.0f);
}