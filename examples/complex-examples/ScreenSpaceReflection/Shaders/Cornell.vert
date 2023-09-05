#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMat;

out vec3 Normal;
out vec3 FragPos;
out vec4 FragPosInLightSpace;
out float Depth;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = transpose(inverse(mat3(model))) * aNormal;
    FragPosInLightSpace = lightSpaceMat * vec4(FragPos, 1.0);
    Depth = gl_Position.z / gl_Position.w;
    return;
}