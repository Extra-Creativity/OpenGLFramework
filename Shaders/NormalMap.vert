#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBiTangent;

out vec2 TexCoords;
out vec3 TangentLightPos;
out vec3 TangentFragPos;
out vec3 TangentViewPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 B = normalize(normalMatrix * aBiTangent);
    vec3 N = normalize(normalMatrix * aNormal);    

    mat3 invTBN = transpose(mat3(T, B, N));
    TangentFragPos = invTBN * vec3(model * vec4(aPos, 1.0));
    TangentLightPos = invTBN * lightPos;
    TangentViewPos = invTBN * viewPos;
    return;
}