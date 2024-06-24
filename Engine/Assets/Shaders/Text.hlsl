// Text.hlsl

Texture2D atlasTexture : register(t0);
SamplerState atlasSampler : register(s0);

cbuffer constants : register(b0)
{
    float4x4 objectToProjection;
    float3 textColor;
};

struct VS_Input
{
    float3 position : POS;
    float2 texCoord : TEX;
};

struct VS_Output
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

//----------------------------- VS_Main -----------------------------

VS_Output VS_Main(VS_Input input)
{
    VS_Output output;
    output.position = mul(objectToProjection, float4(input.position, 1.0f));
    output.texCoord = input.texCoord;
    return output;
}

//----------------------------- PS_Main -----------------------------

float4 PS_Main(VS_Output input) : SV_TARGET
{
    float alpha = atlasTexture.Sample(atlasSampler, input.texCoord);
    float4 color = float4(textColor, alpha);
    return color;
}