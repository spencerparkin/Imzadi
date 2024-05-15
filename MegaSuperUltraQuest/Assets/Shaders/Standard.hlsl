// Standard.hlsl

//----------------------------- VS_Main -----------------------------

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
    output.position = float4(input.pow, 0.0f, 1.0f);
    output.color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    return output;
}

//----------------------------- PS_Main -----------------------------

float4 PS_Main(VS_Output input) : SV_TARGET
{
    return input.color;
}