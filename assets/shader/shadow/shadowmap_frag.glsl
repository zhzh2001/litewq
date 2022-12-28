#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 FragPosLightSpace;

struct PointLight {
    vec3 pos;
    vec3 Ia;
    vec3 Id;
    vec3 Is;
};
uniform PointLight light;

struct Material {
    sampler2D Kd;
    vec3 Ks;
    float highlight_decay;
};

uniform Material material;
uniform sampler2D shadowMap;

uniform vec3 view_pos;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(light.pos - FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

void main()
{
    vec4 color = texture(material.Kd, TexCoords);
    vec3 normal = normalize(Normal);
    vec3 lightColor = vec3(1.0);
    if (color.a < 0.1)
        discard;
    // ambient
    vec3 ambient = color.rgb * light.Ia;
    // diffuse
    vec3 lightDir = light.pos - FragPos;
    float r2 = pow(length(lightDir), 2.0);
    lightDir = normalize(lightDir);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color.rgb * light.Id;

    // specular
    vec3 viewDir = normalize(view_pos - FragPos);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(normal, halfwayDir), 0.0), material.highlight_decay);
    vec3 specular = spec * material.Ks * light.Is;

    // calculate shadow
    float shadow = ShadowCalculation(FragPosLightSpace);
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color.rgb;


    FragColor = vec4(lighting, 1.0);
}
