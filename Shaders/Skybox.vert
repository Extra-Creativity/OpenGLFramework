#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = (aPos - vec3(0.5, 0.5, 0.5)) * 2; // aPos is in [0,1]
    vec4 pos = projection * view * model * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}