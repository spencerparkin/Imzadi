// LegibleText.hlsl

Texture2D atlasTexture : register(t0);
SamplerState atlasSampler : register(s0);

cbuffer constants : register(b0)
{
    float4x4 objectToProjection;
    float3 textForeColor;
    float zFactor;
    float3 textBackColor;
};

struct VS_Input
{
    float2 position : POS;
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
    output.position = mul(objectToProjection, float4(input.position, 0.0f, 1.0f));
    output.position.z *= zFactor;
    output.texCoord = input.texCoord;
    return output;
}

//----------------------------- PS_Main -----------------------------

float4 PS_Main(VS_Output input) : SV_TARGET
{
    float alpha = atlasTexture.Sample(atlasSampler, input.texCoord).a;
    alpha = clamp(alpha, 0.0f, 1.0f);
    float4 color = alpha * float4(textForeColor, 1.0) + (1.0 - alpha) * float4(textBackColor, 1.0);
    return color;
}