#version 330 core

in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosInLightSpace;
in float Depth;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec3 OutNormal;
layout(location = 2) out vec4 WorldPos; // alpha stores depth.

uniform vec3 color;
uniform sampler2D shadowMap;

vec3 projCoords;

void SetProjCoords()
{
    projCoords = FragPosInLightSpace.xyz / FragPosInLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    return;
}

void main()
{
    SetProjCoords();
    if(texture(shadowMap, projCoords.xy).r < projCoords.z - 0.001)
        FragColor = vec4(0, 0, 0, 1);
    else
        FragColor = vec4(color, 1);
    OutNormal = Normal;
    WorldPos = vec4(FragPos, Depth);
    return;
}