#version 330 core

in vec2 texCoord;

uniform sampler2D icons;
uniform float aspect;

out vec4 FragColor;

void main()
{
    float deltaX = 0.15, deltaY = deltaX / aspect;
    if(abs(texCoord.x - 0.5) > deltaX || abs(texCoord.y - 0.5) > deltaY)
    {
        discard;
        return;
    }

    vec2 realTexCoord = vec2((texCoord.x - 0.5 + deltaX) / (2 * deltaX), 
        (texCoord.y - 0.5 + deltaY) / (2 * deltaY));
    vec4 color = texture(icons, realTexCoord);
    if(color.x > 0.66)
    {
        discard;
        return;
    }
    FragColor = vec4(245.0 / 255);
}