#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosInLightSpace;

uniform sampler2D diffuseTexture1;
uniform sampler2D shadowMap;
uniform vec3 lightPos;
uniform vec3 viewPos;

vec3 lightColor = vec3(1.0, 1.0, 1.0);
float ambientCoeff = 0.15;

bool OccludedInShadow(vec4 lightSpacePosition)
{
    vec3 projCoords = lightSpacePosition.xyz / lightSpacePosition.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    return closestDepth < currentDepth - 0.0001;
}

float GetNonAmbientCoeff()
{
    vec3 viewDirection = viewPos - FragPos, 
        lightDirection = lightPos - FragPos;
    float diffuseCoeff = max(
        dot(normalize(viewDirection), normalize(lightDirection)), 0.0
    );

    vec3 halfVec = normalize(viewDirection + lightDirection);
    float specularCoeff = pow(
        max(dot(normalize(Normal), halfVec), 0.0), 32
    );

    return diffuseCoeff + specularCoeff;
}

vec3 GetColorWithHardShadow()
{
    vec3 color = texture(diffuseTexture1, TexCoords).rgb;
    return OccludedInShadow(FragPosInLightSpace) ? color * ambientCoeff: 
         color * lightColor * (ambientCoeff + GetNonAmbientCoeff());
}

void main()
{
    FragColor = vec4(GetColorWithHardShadow(), 1.0);
    return;
}