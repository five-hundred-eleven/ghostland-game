#version 460 core
out vec4 FragColor;

struct Material {
    //sampler2D diffuse;
    //sampler2D specular;
    float shininess;
    vec3 color;
};

struct Light {
    vec3 position;
    vec3 direction;
    float largecutoff;
    float smallcutoff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos;
uniform Material material;
uniform Light light;

void main()
{
    vec3 lightDir = normalize(light.position - FragPos);

    // check if lighting is inside the spotlight cone
    float theta = dot(lightDir, normalize(-light.direction));

    float co = clamp((theta - light.largecutoff) / (light.smallcutoff - light.largecutoff), 0.0, 1.0);
    // diffuse
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * material.color * co;

    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * material.color * co;

    // attenuation
    float distance    = length(light.position - FragPos);
    float atten = 1.0 / (1.0 + 0.1 * distance + 0.01 * (distance * distance));

    diffuse  *= atten;
    specular *= atten;

    vec3 result = light.ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
