#version 400 core
out vec4 fragColor;

in vec3 fragPos;
in vec3 fragNormals;
in vec3 fragColors;

uniform vec3 viewPos;
uniform vec3 lightPos;

void main()
{
    vec3 color = fragColors;
    vec3 norm = normalize(fragNormals);
    vec3 lightDir = normalize(lightPos - fragPos);
    vec3 viewDir = normalize(viewPos - fragPos);

    vec3 ambient = 0.1 * color;
    float diff = max(dot(norm, lightDir), 0.0); // computes the diffuse component of lighting (how much light hits a surface directly)
    // REMEMBER: the dot product of two normalized vectors measures how aligned they are, i.e 1.0 = perfectly facing the light, and 0.0 = perpendicular to the light, 
    // and negative values mean the light is hitting the back of the surface or faced away essentially
    vec3 diffuse = diff * color;

    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = vec3(0.5) * spec;

    vec3 result = ambient + diffuse + specular;
    fragColor = vec4(result, 1.0);
}