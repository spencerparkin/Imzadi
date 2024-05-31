// DebugLine.hlsl

cbuffer constants : register(b0)
{
    float4x4 worldToProjection;
};

struct VS_Input
{
    float3 position : POS;
    float3 color : COL;
};

struct VS_Output
{
    float4 position : SV_POSITION;
    float3 color : COL;
};

//----------------------------- VS_Main -----------------------------

VS_Output VS_Main(VS_Input input)
{
    VS_Output output;
    output.position = mul(worldToProjection, float4(input.position, 1.0f));
    output.color = input.color;
    return output;
}

//----------------------------- PS_Main -----------------------------

float4 PS_Main(VS_Output input) : SV_TARGET
{
    return float4(input.color, 1.0f);
}