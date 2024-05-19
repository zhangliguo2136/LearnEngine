#include "Sampler.hlsl"
#include "Common.hlsl"
#include "Material.hlsl"

TextureCube SkyCubeTexture;

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
	float3 PosL : POSITION;
};

VertexOut VS(VertexIn vin)
{
	VertexOut Out;
	
	Out.PosL = vin.PosL;

	float4x4 View = gView;
	View[3][0] = View[3][1] = View[3][2] = 0.0f;

	float4 PosH = mul(mul(float4(vin.PosL, 1.0f), View), gProj);

	Out.PosH = PosH.xyww;

	return Out;
}

struct PixelOutput
{
	float4 BaseColor		:SV_TARGET0;
	float4 Normal			:SV_TARGET1;
	float4 WorldPos			:SV_TARGET2;
	float4 ORM				:SV_TARGET3;
};

PixelOutput PS(VertexOut pin)
{
	PixelOutput Out;

	Out.BaseColor = SkyCubeTexture.Sample(SamplerLinearWrap, pin.PosL);
	Out.WorldPos.a = (float)cbMaterial.ShadingModel / (float)0xf;

	Out.WorldPos.rgb = 0.0f;
	Out.Normal = 0.f;
	Out.ORM = 0.f;

	return Out;
}