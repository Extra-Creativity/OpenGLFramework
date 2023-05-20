#version 330 core

in vec3 position;
in vec3 normal;
in vec2 texCoords;

uniform sampler2D diffuseTexture1;
uniform samplerCube skybox;
uniform vec3 cameraPos;

out vec4 FragColor;

void main()
{
    vec3 incident = normalize(position - cameraPos);
    vec3 reflection = reflect(incident, normalize(normal));
    FragColor = vec4(texture(skybox, reflection).rgb, 1.0) * 0.5 + 
                        vec4(texture(diffuseTexture1, texCoords).rgb, 1.0) * 0.5;
    return;
}