#version 330 core

// lightProjectionMat * lightViewMat
uniform mat4 lightSpaceMat;
uniform mat4 modelMat;

layout(location = 0) in vec3 aPosition;

void main()
{
    gl_Position = lightSpaceMat * modelMat * vec4(aPosition, 1.0);    
}