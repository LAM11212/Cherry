#version 400 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

struct Material
{
    sampler2D diffuse;
    float shininess;
};

struct DirectionalLight
{
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform DirectionalLight light;

uniform vec3 viewPos;

void main()
{
    // texture color
    vec3 albedo = texture(material.diffuse, TexCoords).rgb;

    // normalize vectors
    vec3 norm = normalize(Normal);

    // directional light direction
    vec3 lightDir = normalize(-light.direction);

    // ambient
    vec3 ambient = light.ambient * albedo;

    // diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * albedo;

    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    float spec = pow(
        max(dot(viewDir, reflectDir), 0.0),
        material.shininess
    );

    vec3 specular = light.specular * spec;
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}