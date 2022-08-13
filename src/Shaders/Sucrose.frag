#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D diffuseTexture1;

void main()
{
    FragColor = texture(diffuseTexture1, TexCoords);    
    return;
}