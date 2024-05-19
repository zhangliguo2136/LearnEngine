
#include "Sampler.hlsl"
#include "PBRLighting.hlsl"

struct VertexIn
{
    float3 PosL : POSITION;
    float2 TexC : TEXCOORD;
};

struct VertexOut
{
    float PosH : SV_POSITION;
    float TexC : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout = (VertexOut) 0.0f;
    
    vout.PosH = float4(vin.PosL, 1.0f);
    vout.TexC = vin.TexC;

    return vout;
}

Texture2D BaseColorGbuffer;
Texture2D NormalGbuffer;
Texture2D WorldPosGbuffer;
Texture2D OrmGbuffer;
Texture2D EmissiveGbuffer;

StructuredBuffer<LightParameters> Lights;

float PS(VertexOut pin):SV_TARGET
{
    float3 FinalColor = 0.0f;
    
    // ShadingModel
    float ShadingModelValue = WorldPosGbuffer.Sample(SamplerPointClamp, pin.TexC).a;
    uint ShadingModel = (uint) round(ShadingModelValue * (float) 0xf);
    
    //GBuffer
    float3 BaseColor = BaseColorGbuffer.Sample(SamplerPointClamp, pin.TexC).rgb;
    float3 Normal = NormalGbuffer.Sample(SamplerPointClamp, pin.TexC).rgb;
    float3 WorldPos = WorldPosGbuffer.Sample(SamplerPointClamp, pin.TexC).rgb;
    
    
    float Roughness = OrmGbuffer.Sample(SamplerPointClamp, pin.TexC).g;
    float Metallic = OrmGbuffer.Sample(SamplerPointClamp, pin.TexC).b;
    
    // 自发光颜色
    float EmissiveColor = EmissiveGbuffer.Sample(SamplerPointClamp, pin.TexC).rgb;
    
    // DefaultLit
    if (ShadingModel == 0)
    {
        FinalColor += EmissiveColor;
        
        for (uint i = 0; i < LightCount; i++)
        {
            LightParameters Light = Lights[i];
            
            // Directional Light
            if (Light.LightType == 1)
            {
                float3 ViewDir = float3(0.f);
                
                float3 LightDir = normalize(-Light.Direction);
                float3 LightPosition = WorldPos + LightDir * 100.f;
                
                // 方向光没有Attenuation(衰减)
                float3 Radiance = Light.Intensity * Light.Color;

                FinalColor += DirectLighting(Radiance, LightDir, ViewDir, Normal, BaseColor, Roughness, Metallic);
            }
        }

    }
    // UnLit
    else if (ShadingModel == 1)
    {
        FinalColor = BaseColor;
    }

    return float4(FinalColor, 1.0f);

}