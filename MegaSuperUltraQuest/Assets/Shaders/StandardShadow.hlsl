// This shader is for shadowing fully opaque objects in the scene.

cbuffer constants : register(b0)
{
    float4x4 objectToProjection;
};

struct VS_Input
{
    float3 position : POS;
};

struct VS_Output
{
    float4 position : SV_POSITION;
};

//----------------------------- VS_Main -----------------------------

VS_Output VS_Main(VS_Input input)
{
    VS_Output output;
    output.position = mul(objectToProjection, float4(input.position, 1.0f));
    return output;
}

//----------------------------- PS_Main -----------------------------

float PS_Main(VS_Output input) : SV_DEPTH
{
    return input.position.z;
}