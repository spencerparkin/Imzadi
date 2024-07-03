// Standard.hlsl

// Note that we're not doing any alpha-blending/transparency here.
// This shader is for fully opaque objects in the scene.

Texture2D diffuseTexture : register(t0);
SamplerState diffuseSampler : register(s0);

Texture2D shadowTexture : register(t1);
SamplerState shadowSampler : register(s1);

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
    
    // These are additional variables needed for the shadow calculations.
    float3 lightCameraEyePoint;
    float3 lightCameraXAxis;
    float3 lightCameraYAxis;
    float lightCameraWidth;
    float lightCameraHeight;
    float lightCameraNear;
    float lightCameraFar;
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
    float3 worldPosition : POS;
};

//----------------------------- VS_Main -----------------------------

VS_Output VS_Main(VS_Input input)
{
    VS_Output output;
    output.position = mul(objectToProjection, float4(input.position, 1.0f));
    output.texCoord = input.texCoord;
    output.normal = mul(objectToWorld, float4(input.normal, 0.0)).xyz;  // We are assuming no shear or scale here.
    output.worldPosition = mul(objectToWorld, float4(input.position, 1.0)).xyz;
    return output;
}

//----------------------------- PS_Main -----------------------------

float4 PS_Main(VS_Output input) : SV_TARGET
{
    float shadowFactor = 1.0;
    if(dot(lightDirection, input.normal) < -0.1)
    {
        float lambda = dot(lightCameraEyePoint - input.worldPosition, lightDirection);
        float3 lightCameraPoint = input.worldPosition + lambda * lightDirection;
        float lightCameraPointX = dot(lightCameraPoint - lightCameraEyePoint, lightCameraXAxis);
        float lightCameraPointY = dot(lightCameraPoint - lightCameraEyePoint, lightCameraYAxis);
        float2 lightCameraUVs;
        lightCameraUVs.x = lightCameraPointX / lightCameraWidth + 0.5;
        lightCameraUVs.y = -lightCameraPointY / lightCameraHeight + 0.5;
        if (0.0 <= lightCameraUVs.x && lightCameraUVs.x <= 1.0 &&
            0.0 <= lightCameraUVs.y && lightCameraUVs.y <= 1.0)
        {
            float depth = (float)shadowTexture.Sample(shadowSampler, lightCameraUVs);
            float shadowBufferDistance = lightCameraNear + depth * (lightCameraFar - lightCameraNear);
            float surfacePointDistance = abs(lambda);
            float tolerance = 0.5;
            if(shadowBufferDistance + tolerance < surfacePointDistance)
                shadowFactor = 0.5;
        }
    }

    float3 lightReflectionDirection = lightDirection - 2.0 * dot(lightDirection, input.normal) * input.normal;
    float3 directionToViewer = normalize(cameraEyePoint - input.position.xyz);
    float4 diffuseColor = diffuseTexture.Sample(diffuseSampler, input.texCoord);
    
    // This is my approximation of the Phong lighting model.  Colors here are
    // all properties of the surface material, and I think I'm only assuming
    // we have white light here.
    float4 color = float4(0.0, 0.0, 0.0, 1.0);
    
    // Add the ambient component.
    color += ambientLightIntensity * diffuseColor;
    
    // Add the diffuse component.
    color += directionalLightIntensity * clamp(dot(-lightDirection, input.normal), 0.0, 1.0) * diffuseColor;
    
    // Add the specular component.
    color += pow(clamp(dot(directionToViewer, lightReflectionDirection), 0.0, 1.0), shininessExponent) * float4(1.0, 1.0, 1.0, 0.0);
    
    // Lastly, account for the shadow factor.
    color *= shadowFactor;
    
    // We're always opaque.
    color.a = 1.0;
    
    return color;
}