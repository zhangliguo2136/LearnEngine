#include "Sampler.hlsl"
#include "Material.hlsl"
#include "Common.hlsl"
#include "Utils.hlsl"

Texture2D BaseColorTexture;
Texture2D NormalTexture;
Texture2D MetallicTexture;
Texture2D RoughnessTexture;

struct VertexIn
{
    float3 PosL         : POSITION;
    float3 NormalL      : NORMAL;
    float3 TangentU     : TANGENT;
    float2 TexC         : TEXCOORD;
};

struct VertexOut
{
    float4 PosH         : SV_POSITION;
    float4 CurPosH      : POSITION0;
    float4 PrevPosH     : POSITION1;
    float3 PosW         : POSITION2;
    float3 NormalW      : NORMAL;
    float2 TexC         : TEXCOORD;
    float3 TangentW     : TANGENT;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout = (VertexOut) 0.0f;

    // Transform to world space.
    float4 PosW = mul(float4(vin.PosL, 1.0f), gWorld);
    vout.PosW = PosW.xyz;

    // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
    vout.NormalW = mul(vin.NormalL, (float3x3)gWorld);

    vout.TangentW = mul(vin.TangentU, (float3x3)gWorld);

    // Transform to homogeneous clip space.
    vout.PosH = mul(PosW, gViewProj);

    // CurPosH and PrevPosH
    vout.CurPosH = mul(PosW, gViewProj);

    float4 PrevPosW = mul(float4(vin.PosL, 1.0f), gPrevWorld);
    vout.PrevPosH = mul(PrevPosW, gPrevViewProj);

    vout.TexC = vin.TexC;

    return vout;
}

struct PixelOutput
{
    float4 BaseColor : SV_TARGET0;
    float4 Normal : SV_TARGET1;
    float4 WorldPos : SV_TARGET2;
    float4 ORM : SV_TARGET3;
    float2 Velocity : SV_TARGET4;   // 速度在TAA时使用
    float4 Emissive : SV_TARGET5;
};

PixelOutput PS(VertexOut pin) //: SV_TARGET
{
    PixelOutput Out = (PixelOutput)1.0f;

    // 基础色
    Out.BaseColor = BaseColorTexture.Sample(SamplerAnisotropicWrap, pin.TexC);
    
    // 法线
    float4 NormalMapSample = NormalTexture.Sample(SamplerAnisotropicWrap, pin.TexC);
    float3 Normal = NormalSampleToWorldSpace(NormalMapSample.rgb, pin.NormalW, pin.TangentW);
    Out.Normal = float4(normalize(Normal), 1.0f);
    
    // 世界位置
    Out.WorldPos = float4(pin.PosW, 1.0f);
    
    // ShadingModel
    Out.WorldPos.a = (float)cbMaterial.ShadingModel / (float)0xF;
    
    // 金属度
    float Metallic = MetallicTexture.Sample(SamplerAnisotropicWrap, pin.TexC).r;
    // 粗糙度
    float Roughness = RoughnessTexture.Sample(SamplerAnisotropicWrap, pin.TexC).r;
    
    Out.ORM = float4(0.f, Roughness, Metallic, 0.f);
    
    // 计算速度
    float4 CurPos = pin.CurPosH;
    CurPos /= CurPos.w;
    CurPos.xy = NDCToUV(CurPos);
    
    float4 PrevPos = pin.PrevPosH;
    PrevPos /= PrevPos.w;
    PrevPos.xy = NDCToUV(PrevPos);

    // 速度
    Out.Velocity = CurPos.xy - PrevPos.xy;
    
    // 自发光
    Out.Emissive = float4(cbMaterial.EmissiveColor, 1.0f);

    //float4 Out = 1.0f;
    return Out;
}