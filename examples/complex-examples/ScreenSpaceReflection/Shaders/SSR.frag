#version 330 core

#define PI 3.1415926535897932384626433832795
#define TWO_PI 6.283185307
#define INV_PI 0.31830988618
#define INV_TWO_PI 0.15915494309

in vec2 texCoord;

uniform sampler2D DirectIllum;
uniform sampler2D WorldNormal;
uniform sampler2D WorldPosAndDepth; // alpha stores depth.

uniform vec3 LightPos;
uniform mat4 ScreenMat;
uniform int Option;

out vec4 FragColor;

vec3 lightRadiance = vec3(1);

vec3 GetColor(vec2 uv){ return texture(DirectIllum, uv).xyz; }
vec3 GetWorldNormal(vec2 uv){ return texture(WorldNormal, uv).xyz; }
vec3 GetWorldPos(vec2 uv){ return texture(WorldPosAndDepth, uv).xyz; }
float GetDepth(vec2 uv){ return texture(WorldPosAndDepth, uv).w; }

vec2 WorldPosToScreenPos(vec3 pos, out float depth)
{
    vec4 newPos = ScreenMat * vec4(pos, 1);
    newPos /= newPos.w;
    depth = newPos.z;
    return newPos.xy * 0.5 + 0.5;
}

bool RayMarch(vec3 beginPos, vec3 outDir, out vec3 hitPos)
{
    float depth;
    for(int i = 0; i < 100; i++)
    {
        vec3 newPos = beginPos + outDir * i * 0.1;
        vec2 screenPos = WorldPosToScreenPos(newPos, depth);
        float newDepth = GetDepth(screenPos);
        if(depth > newDepth * 1.01)
        {
            hitPos = newPos;
            return true;
        }
    }
    return false;
}

float Rand1(inout float p) {
    p = fract(p * .1031);
    p *= p + 33.33;
    p *= p + p;
    return fract(p);
}

vec2 Rand2(inout float p) {
    return vec2(Rand1(p), Rand1(p));
}

float GetRandSeed(vec2 uv) {
    vec3 p3  = fract(vec3(uv.xyx) * .1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

void FrisvadONB(vec3 vec, out vec3 xAxis, out vec3 yAxis) {
    if (vec.z < -0.9999) // Handle the singularity
    {
        xAxis = vec3(0, -1, 0), yAxis = vec3(-1, 0, 0);
        return;
    }
    float x = vec.x, y = vec.y, z = vec.z;
    float a = 1.0f / (1.0f + z), b = -x * y * a;

    xAxis = vec3(1.0f - x * x * a, b, -x);
    yAxis = vec3(b, 1.0f - y * y * a, -y);
};

vec3 SampleHemisphereUniform(inout float s, out float pdf) {
    vec2 uv = Rand2(s);
    float z = uv.x;
    float phi = uv.y * TWO_PI;
    float sinTheta = sqrt(1.0 - z*z);
    vec3 dir = vec3(sinTheta * cos(phi), sinTheta * sin(phi), z);
    pdf = INV_TWO_PI;
    return dir;
}

vec3 EvalDiffuse(vec3 wi, vec3 wo, vec2 uv)
{
    vec3 albedo = GetColor(uv);
    vec3 worldNormal = GetWorldNormal(uv);
    return albedo * max(0, dot(worldNormal, wi)) * INV_PI;
}

#define SAMPLE_NUM 10

vec3 GetIllum(float seed)
{
    vec3 worldPos = GetWorldPos(texCoord);
    vec3 wi = normalize(LightPos - worldPos), wo = vec3(1);
    vec3 L_d = EvalDiffuse(wi, wo, texCoord) * lightRadiance;
    if(Option == 0) // Only direct illum.
        return L_d;

    vec3 worldNormal = GetWorldNormal(texCoord);

    if(Option == 1)  // Direct & Specular.
    {
        vec3 hitPos; float _;
        if(RayMarch(worldPos, reflect(normalize(worldPos - LightPos), worldNormal), hitPos))
            return EvalDiffuse(wi, wo, WorldPosToScreenPos(hitPos, _)) * lightRadiance + L_d;
    }

    // Direct & Diffuse.
    vec3 L_ind = vec3(0);
    vec3 x, y;
    FrisvadONB(worldNormal, x, y);
    for(int i = 0; i < SAMPLE_NUM; i++){
        float pdf;
        vec3 localDir = SampleHemisphereUniform(seed, pdf);
        vec3 dir = normalize(mat3(x, y, worldNormal) * localDir);

        vec3 hitPos; float _;
        if(RayMarch(worldPos, dir, hitPos)){
            vec2 hitScreenUV = WorldPosToScreenPos(hitPos, _);
            L_ind += EvalDiffuse(dir, wo, texCoord) / pdf * EvalDiffuse(wi, dir, hitScreenUV) * GetColor(hitScreenUV) * lightRadiance;
        }
    }
    L_ind /= float(SAMPLE_NUM);
    return L_ind + L_d;
}

void main()
{
    if(GetDepth(texCoord) > 0.99)
    { discard; return; }

    float seed = GetRandSeed(texCoord);
    FragColor = vec4(GetIllum(seed), 1);
    return;
}