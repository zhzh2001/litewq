#version 330 core
out vec4 frag_color;

struct Material {
    sampler2D Kd;
    vec3 Ks; // specular coeff.
    float highlight_decay; // control the size of highlight.
};
in vec2 frag_tex_coord;

struct PointLight {
    vec3 pos;
    vec3 Ia;
    vec3 Id;
    vec3 Is;
};


in vec3 frag_pos;
in vec3 frag_normal;

uniform vec3 view_pos;
uniform Material material;
uniform PointLight light;

void main() {
    vec4 DiffuseMapTexColor = texture(material.Kd, frag_tex_coord);
    if(DiffuseMapTexColor.a < 0.1)
       discard;
    // ambient
    vec3 La = DiffuseMapTexColor.rgb * light.Ia;
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