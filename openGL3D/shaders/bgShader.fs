#version 400 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D bgTexture;

void main() 
{
    FragColor = texture(bgTexture, TexCoord);
}