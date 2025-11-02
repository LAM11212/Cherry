#version 400 core
out vec4 fragColor;

in vec3 fragPos;  
in vec3 Normal;  
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
  
uniform vec3 viewPos;
uniform vec3 lightPos;

void main()
{
    vec3 color = texture(texture_diffuse1, TexCoords).rgb;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - fragPos);
    vec3 viewDir = normalize(viewPos - fragPos);

    vec3 ambient = 0.15 * color;

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * color;

    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = vec3(0.5) * spec;

    vec3 result = ambient + diffuse + specular;
    fragColor = vec4(result, 1.0);
} 