// SkyDome.hlsl

TextureCube cubeTexture : register(t0);
SamplerState cubeSampler : register(s0);

cbuffer constants : register(b0)
{
    float4x4 objectToProjection;
}

struct VS_Input
{
    float3 position : POS;
};

struct VS_Output
{
    float3 objPos : POS;
    float4 projPos : SV_POSITION;
};

//----------------------------- VS_Main -----------------------------

VS_Output VS_Main(VS_Input input)
{
    VS_Output output;
    output.objPos = input.position;
    output.projPos = mul(objectToProjection, float4(input.position, 1.0f));
    return output;
}

//----------------------------- PS_Main -----------------------------

float4 PS_Main(VS_Output input) : SV_TARGET
{
    float3 texCoordVec = normalize(input.objPos);
    float4 color = cubeTexture.Sample(cubeSampler, texCoordVec);
    return color;
}