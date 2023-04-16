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
uniform int shadowOption;

vec3 lightColor = vec3(1.0, 1.0, 1.0);
float ambientCoeff = 0.15;

bool OccludedInShadow(vec2 offset, bool needBias)
{
    vec3 projCoords = FragPosInLightSpace.xyz / FragPosInLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy + offset).r;
    float currentDepth = projCoords.z;
    return needBias ? closestDepth < currentDepth - 0.0001 :
        closestDepth < currentDepth;
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

vec3 GetColorWithHardShadow(bool needBias)
{
    vec3 color = texture(diffuseTexture1, TexCoords).rgb;
    return OccludedInShadow(vec2(0.0), needBias) ? color * ambientCoeff: 
        color * lightColor * (ambientCoeff + GetNonAmbientCoeff());
}

vec3 GetColorWithPCFShadow()
{
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    int validCnt = 0, sampleLen = 7, range = sampleLen / 2;

    for(int i = -range; i <= range; i++)
    {
        for(int j = -range; j <= range; j++)
        {
            if(!OccludedInShadow(vec2(i, j) * texelSize, true))
                validCnt++;
        }
    }
    vec3 color = texture(diffuseTexture1, TexCoords).rgb;
    return color * lightColor * (ambientCoeff + 
        validCnt / float(sampleLen * sampleLen) * GetNonAmbientCoeff());
}

vec3 GetColorWithPCSS()
{
    return vec3(0.0);
}

void main()
{
    vec3 resultColor = vec3(0.0);
    if(shadowOption == 0)
        resultColor = GetColorWithHardShadow(false);
    else if(shadowOption == 1)
        resultColor = GetColorWithHardShadow(true);
    else if(shadowOption == 2)
        resultColor = GetColorWithPCFShadow();
    else if(shadowOption == 3)
        resultColor = GetColorWithPCSS();

    FragColor = vec4(resultColor, 1.0);
    return;
}