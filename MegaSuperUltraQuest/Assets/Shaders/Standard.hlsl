// Standard.hlsl

// Note that we're not doing any alpha-blending/transparency here.
// This shader is for fully opaque objects in the scene.

Texture2D diffuseTexture : register(t0);
SamplerState diffuseSampler : register(s0);

//----------------------------- VS_Main -----------------------------

// Note that no variable will straddle a 16-byte boundary,
// so we have to take that into account when calculating
// sizes and offsets.
cbuffer constants : register(b0)
{    
    float4x4 objectToProjection;
    float4x4 objectToWorld;
    
    // Note that I'm using a directional light source, so there
    // is no position of the light source.  This direction should
    // be of unit-length and points from the source toward the scene.
    float3 lightDirection;
    
    // These intensity range in [0,1].
    float directionalLightIntensity;
    float ambientLightIntensity;
    float shininessExponent;
    
    // This is the world location of the camera view point.
    float3 cameraEyePoint;
};

struct VS_Input
{
    float3 position : POS;
    float2 texCoord : TEX;
    float3 normal : NORM;
};

struct VS_Output
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORM;
};

VS_Output VS_Main(VS_Input input)
{
    VS_Output output;
    output.position = mul(objectToProjection, float4(input.position, 1.0f));
    output.position /= output.position.w;
    output.texCoord = input.texCoord;
    output.normal = mul(objectToWorld, float4(input.normal, 0.0)).xyz;  // We are assuming no shear or scale here.
    return output;
}

//----------------------------- PS_Main -----------------------------

float4 PS_Main(VS_Output input) : SV_TARGET
{
    float3 lightReflectionDirection = lightDirection - 2.0 * dot(lightDirection, input.normal) * input.normal;
    float3 directionToViewer = normalize(cameraEyePoint - input.position.xyz);
    float4 diffuseColor = diffuseTexture.Sample(diffuseSampler, input.texCoord);
    
    // This is my approximation of the Phong lighting model.  Colors here are
    // all properties of the surface material, and I think I'm only assuming
    // we have white light here.
    // TODO: Factor a shadow-map into this calculation once we have that ready.
    float4 color = float4(0.0, 0.0, 0.0, 1.0);
    
    // Add the ambient component.
    color += ambientLightIntensity * diffuseColor;
    
    // Add the diffuse component.
    color += directionalLightIntensity * clamp(dot(-lightDirection, input.normal), 0.0, 1.0) * diffuseColor;
    
    // Add the specular component.
    color += pow(clamp(dot(directionToViewer, lightReflectionDirection), 0.0, 1.0), shininessExponent) * float4(1.0, 1.0, 1.0, 0.0);
    
    // We're always opaque.
    color.a = 1.0;
    
    return color;
}