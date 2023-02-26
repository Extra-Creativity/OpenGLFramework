#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D diffuseTexture1;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 cameraPos;

void main()
{
    vec4 baseColor = texture(diffuseTexture1, TexCoords);

    float ks = 100, kd = 50, ka = 0.1;
    int p = 32;

    vec3 shadePointToLightVec = lightPos - FragPos;
    float attenuateDistance = length(shadePointToLightVec);
    vec4 backgroundColor = vec4(1.0, 1.0, 1.0, 1.0);

    vec3 inVec = normalize(cameraPos - FragPos);
    vec3 outVec = normalize(shadePointToLightVec);

    vec3 halfVec = normalize(inVec + outVec);
    float specularCoeff = ks * pow(max(0, dot(halfVec, Normal)), p);
    float diffuseCoeff = kd * max(0, dot(inVec, outVec));

    FragColor = ((specularCoeff + diffuseCoeff) / 
                (attenuateDistance * attenuateDistance)) * baseColor * 
                vec4(lightColor, 1.0) + backgroundColor * ka;
    return;
}