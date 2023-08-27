#version 330 core
in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

layout(location = 0) out vec3 aPos;
layout(location = 1) out vec3 aNormal;
layout(location = 2) out vec4 FragColor;
layout(location = 3) out float depth;

uniform sampler2D diffuseTexture1;

void main()
{
    aPos = FragPos;
    aNormal = Normal;
    FragColor = vec4(texture2D(diffuseTexture1, TexCoords));
    // depth is near 1 since non-linear depth transformation;
    // You may see how to correct it in DepthCorrectionDraw.frag
    depth = gl_FragCoord.z;
    return;
}