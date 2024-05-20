// Box.hlsl

//----------------------------- VS_Main -----------------------------

cbuffer constants : register(b0)
{
    float4x4 objectToProjection;
    float4 color;
    float3 lightDirection;
    float lightIntensity;
    float4 lightColor;
};

struct VS_Input
{
    float3 pos : POS;
    float2 texCoord : TEX;
    float3 normal : NORM;
};

struct VS_Output
{
    float4 position : SV_POSITION;
    float4 color : COL;
    float3 normal : NORM;
};

VS_Output VS_Main(VS_Input input)
{
    VS_Output output;
    output.position = mul(objectToProjection, float4(input.pos, 1.0f));
    output.position /= output.position.w;
    output.color = color;
    output.normal = input.normal;   // TODO: Transform object to world.
    return output;
}

//----------------------------- PS_Main -----------------------------

float4 PS_Main(VS_Output input) : SV_TARGET
{
    /*float pi = 3.1415926536;
    float angle = acos(dot(lightDirection, input.normal));
    float lightFactor = angle / pi;
    return input.color * lightFactor;*/
    
    return input.color;
}