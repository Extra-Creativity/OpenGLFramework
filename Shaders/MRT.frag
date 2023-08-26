#version 330 core
in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

layout(location = 0) out vec3 aPos;
layout(location = 1) out vec3 aNormal;
layout(location = 2) out vec4 FragColor;

uniform sampler2D diffuseTexture1;

void main()
{
    aNormal = Normal;
    aPos = FragPos;
    FragColor = vec4(texture2D(diffuseTexture1, TexCoords));
    return;
}