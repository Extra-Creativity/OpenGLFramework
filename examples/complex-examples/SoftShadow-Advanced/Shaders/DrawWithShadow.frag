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
vec3 projCoords;

// Legs have artifacts now; if goes up to 50, soft shadow appears there.
#define LIGHT_WIDTH 20
#define BLOCKER_SEARCH_WIDTH 5

void SetProjCoords()
{
    projCoords = FragPosInLightSpace.xyz / FragPosInLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    return;
}

float GetClosestDepth(vec2 offset)
{
    return texture(shadowMap, projCoords.xy + offset).r;
}

bool OccludedInShadow(vec2 offset, bool needBias)
{
    float closestDepth = GetClosestDepth(offset);
    float currentDepth = projCoords.z;
    return needBias ? closestDepth < currentDepth - 0.0125 :
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

vec3 GetNonOcculdedColor()
{
    vec3 color = texture(diffuseTexture1, TexCoords).rgb;
    return color * lightColor * (ambientCoeff + GetNonAmbientCoeff());
}

vec3 GetColorWithHardShadow(bool needBias)
{
    return OccludedInShadow(vec2(0.0), needBias) ? 
        texture(diffuseTexture1, TexCoords).rgb * ambientCoeff: 
        GetNonOcculdedColor();
}

vec3 GetColorWithPCFShadow(int range)
{
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    int validCnt = 0;

    for(int i = -range; i <= range; i++)
    {
        for(int j = -range; j <= range; j++)
        {
            if(!OccludedInShadow(vec2(i, j) * texelSize, true))
                validCnt++;
        }
    }

    range = range * 2 + 1;
    float area = range * range;
    vec3 color = texture(diffuseTexture1, TexCoords).rgb;
    return color * lightColor * (ambientCoeff + 
        validCnt / area * GetNonAmbientCoeff());
}

float GetAvgBlockerDepth()
{
    float currentDepth = projCoords.z;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    float result = 0.0;
    int range = BLOCKER_SEARCH_WIDTH / 2;
    int validCnt = 0;
    for(int i = -range; i <= range; i++)
        for(int j = -range; j <= range; j++)
        {
            float depth = GetClosestDepth(vec2(i, j) * texelSize);
            if(depth >= currentDepth)
                continue;
            result += GetClosestDepth(vec2(i, j) * texelSize);
            validCnt++;
        }
    return validCnt == 0 ? currentDepth : result / float(validCnt);
}

highp vec3 GetColorWithPCSS()
{
    float blockerDepth = GetAvgBlockerDepth();
    float currentDepth = projCoords.z;
    float sampleWidth = float(LIGHT_WIDTH) *
        (currentDepth - blockerDepth) / blockerDepth;
    int range = int(sampleWidth / 2);
    return GetColorWithPCFShadow(range);
}

void main()
{
    SetProjCoords();
    vec3 resultColor = vec3(0.0);
    if(shadowOption == 0)
        resultColor = GetColorWithHardShadow(false);
    else if(shadowOption == 1)
        resultColor = GetColorWithHardShadow(true);
    else if(shadowOption == 2)
        resultColor = GetColorWithPCFShadow(3); // sampleWidth = 3 * 2 + 1
    else if(shadowOption == 3)
        resultColor = GetColorWithPCSS();

    FragColor = vec4(resultColor, 1.0);
    return;
}