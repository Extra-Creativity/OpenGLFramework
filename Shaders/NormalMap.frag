#version 330 core
in vec2 TexCoords;
in vec3 TangentLightPos;
in vec3 TangentFragPos;
in vec3 TangentViewPos;

uniform sampler2D diffuseTexture1;
uniform sampler2D normalMap;

out vec4 FragColor;

void main()
{
    vec3 normal = vec3(texture2D(normalMap, TexCoords));
    normal = normalize(normal * 2.0 - 1.0);
    vec4 baseColor = texture2D(diffuseTexture1, TexCoords); 

    float ks = 50, kd = 25, ka = 0.1;
    int p = 32;

    vec3 viewVec = TangentViewPos - TangentFragPos;
    vec3 lightVec = TangentLightPos - TangentFragPos;

    float attenuateDistance = length(lightVec);
    vec4 backgroundColor = vec4(1.0, 1.0, 1.0, 1.0);

    vec3 inVec = normalize(viewVec);
    vec3 outVec = normalize(lightVec);

    vec3 halfVec = normalize(inVec + outVec);
    float specularCoeff = ks * pow(max(0, dot(halfVec, normal)), p);
    float diffuseCoeff = kd * max(0, dot(inVec, outVec));

    FragColor = ((specularCoeff + diffuseCoeff) / 
                (attenuateDistance * attenuateDistance)) * baseColor
                + backgroundColor * ka;
}