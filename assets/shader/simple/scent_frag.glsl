#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture1;

uniform vec3 center;
uniform vec3 outer;

void main() 
{
    vec3 color = vec3(0.95, 0.54, 0.21);
    vec2 uv = gl_FragCoord.xy - center.xy;
    float dist = length(uv);
    float radius = length(outer - center);
    float alpha = 1.0 - smoothstep(0, radius, dist);
    FragColor = vec4(color, alpha);
}