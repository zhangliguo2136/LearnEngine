#include "Sampler.hlsl"
#include "Material.hlsl"
#include "Common.hlsl"

Texture2D BaseColorTexture;
Texture2D NormalTexture;
Texture2D MetallicTexture;
Texture2D RoughnessTexture;

struct VertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 TexC : TEXCOORD;
    float3 TangentU : TANGENT;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float4 CurrPosH : POSITION0;
    float4 PrevPosH : POSITION1;
    float3 PosW : POSITION2;
    float3 NormalW : NORMAL;
    float2 TexC : TEXCOORD;
    float3 TangentW : TANGENT;
};

VertexOut VS(VertexIn vin)
{
    VertexOut Out = (VertexOut) 0.0f;

    return Out;
}

struct PixelOutput
{
    float4 BaseColor : SV_TARGET0;
    float4 Normal : SV_TARGET1;
    float4 WorldPos : SV_TARGET2;
    float4 ORM : SV_TARGET3;
    float2 Velocity : SV_TARGET4;   // �ٶ���TAAʱʹ��
    float4 Emissive : SV_TARGET5;
};

PixelOutput PS(VertexOut pin)
{
    PixelOutput Out;
    
    // ����ɫ
    Out.BaseColor = BaseColorTexture.Sample(SamplerAnisotropicWrap, pin.TexC);
    
    // ����
    Out.Normal = NormalTexture.Sample(SamplerAnisotropicWrap, pin.TexC);
    
    // ����λ��
    Out.WorldPos = float4(pin.PosW, 1.0f);
    
    // ShadingModel
    Out.WorldPos.a = (float) cbMaterial.ShadingModel / (float) 0xF;
    
    // ������
    float Metallic = 0.0f;
    // �ֲڶ�
    float Roughness = 0.64f;
    Out.ORM = float4(0.f, Roughness, Metallic, 0.f);
    
    // �����ٶ�
    float4 CurPos = pin.CurrPosH;
    CurPos /= CurPos.w;
    //CurPos.xy = NDCToUV(CurPos);
    
    float4 PrevPos = pin.PrevPosH;
    PrevPos /= PrevPos.w;
    //PrevPos.xy = NDCToUV(PrevPos);
    // �ٶ�
    Out.Velocity = CurPos.xy - PrevPos.xy;
    
    // �Է���
    Out.Emissive = float4(cbMaterial.EmissiveColor, 1.0f);
    
    return Out;
}