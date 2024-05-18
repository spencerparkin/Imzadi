// Box.hlsl

//----------------------------- VS_Main -----------------------------

cbuffer constants : register(b0)
{
    float4x4 objectToProjection;
    float4 color;
};

struct VS_Input
{
    float3 pos : POS;
};

struct VS_Output
{
    float4 position : SV_POSITION;
    float4 color : COL;
};

VS_Output VS_Main(VS_Input input)
{
    VS_Output output;
    output.position = mul(objectToProjection, float4(input.pos, 1.0f));
    output.position /= output.position.w;
    output.color = color;
    return output;
}

//----------------------------- PS_Main -----------------------------

float4 PS_Main(VS_Output input) : SV_TARGET
{
    return input.color;
}