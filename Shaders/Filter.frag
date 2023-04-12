#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D colorTexture;
uniform sampler2D depthTexture;
uniform int filterOption;
uniform float xOffset;
uniform float yOffset;
uniform	float pixelSigmaSqr;
uniform	float depthSigmaSqr;
uniform	float colorSigmaSqr;

vec2 offsets[9] = vec2[](
        vec2(-xOffset,  yOffset), // 左上
        vec2( 0.0f,    yOffset), // 正上
        vec2( xOffset,  yOffset), // 右上
        vec2(-xOffset,  0.0f),   // 左
        vec2( 0.0f,    0.0f),   // 中
        vec2( xOffset,  0.0f),   // 右
        vec2(-xOffset, -yOffset), // 左下
        vec2( 0.0f,   -yOffset), // 正下
        vec2( xOffset, -yOffset)  // 右下
    );

vec3 GaussianFilter()
{
    // assume gaussian_sigma^2 = 0.5;
    float coeff = exp(-1 / pixelSigmaSqr);
    vec3 color = vec3(0.0);
    for(int i = 0; i < 4; i++)
    {
        color += texture(colorTexture, TexCoords + offsets[i]).xyz * coeff;
    }
    color += texture(colorTexture, TexCoords).xyz;
    for(int i = 5; i < 9; i++)
    {
        color += texture(colorTexture, TexCoords + offsets[i]).xyz * coeff;
    }
    color = color / (coeff * 8 + 1);
    return color;
}

vec3 JointBilateralFilter()
{
    vec3 color = vec3(0.0);
    vec3 weightSum = vec3(0.0);

    float coeff1 = -1 / pixelSigmaSqr, coeff2 = -1 / (2 * depthSigmaSqr),
        coeff3 = -1 / (2 * colorSigmaSqr);
    vec3 centerColor = texture(colorTexture, TexCoords).xyz;
    float centerDepth = texture(depthTexture, TexCoords).x;
    for(int i = 0; i < 4; i++)
    {
        vec2 currCoord = TexCoords + offsets[i];
        vec3 currColor = texture(colorTexture, currCoord).xyz;
        float currDepth = texture(depthTexture, currCoord).x;
        vec3 weight = exp(coeff1 + abs(centerDepth - currDepth) * coeff2 + 
            abs(centerColor - currColor) * coeff3);
        color += currColor * weight;
        weightSum += weight;
    }
    color += centerColor;
    for(int i = 5; i < 9; i++)
    {
        vec2 currCoord = TexCoords + offsets[i];
        vec3 currColor = texture(colorTexture, currCoord).xyz;
        float currDepth = texture(depthTexture, currCoord).x;
        vec3 weight = exp(coeff1 + abs(centerDepth - currDepth) * coeff2 + 
            abs(centerColor - currColor) * coeff3);
        color += currColor * weight;
        weightSum += weight;
    }
    color = color / (weightSum + 1);
    return color;
}

void main()
{
    if(filterOption == 0)
        FragColor = vec4(texture(colorTexture, TexCoords).xyz, 1.0);
    else if(filterOption == 1)
        FragColor = vec4(GaussianFilter(), 1.0);
    else if(filterOption == 2)
        FragColor = vec4(JointBilateralFilter(), 1.0);
    return;
}