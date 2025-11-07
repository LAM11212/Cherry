#version 400 core
out vec4 fragColor;

struct Material {
    sampler2D diffuse; // this is our texture
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 fragPos;
in vec3 fragNormals;
in vec2 texCoords;

uniform vec3 viewPos;
uniform Material material;
uniform Light light;

void main()
{
    vec3 ambient = light.ambient * texture(material.diffuse, texCoords).rgb; // this is our ambient
    vec3 norm = normalize(fragNormals);
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 viewDir = normalize(viewPos - fragPos);

    float diff = max(dot(norm, lightDir), 0.0); // computes the diffuse component of lighting (how much light hits a surface directly)
    // REMEMBER: the dot product of two normalized vectors measures how aligned they are, i.e 1.0 = perfectly facing the light, and 0.0 = perpendicular to the light, 
    // and negative values mean the light is hitting the back of the surface or faced away essentially
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, texCoords).rgb;

    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);

    vec3 result = ambient + diffuse + specular;
    fragColor = vec4(result, 1.0);
}