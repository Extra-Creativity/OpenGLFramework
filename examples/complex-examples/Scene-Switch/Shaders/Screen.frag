#version 330 core

in vec2 texCoord;

uniform float border;

out vec4 FragColor;

void main()
{
    if(texCoord.x < border)
        FragColor = vec4(100.0 / 255);
    else
        FragColor = vec4(245.0 / 255);
    return;
}