#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D diffuseTexture1;

void main()
{
    FragColor = vec4(texture(diffuseTexture1, TexCoords).rrr, 1.0);    
    return;
}