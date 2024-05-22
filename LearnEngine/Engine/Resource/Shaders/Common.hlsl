#ifndef __SHADER_COMMON__
#define __SHADER_COMMON__

cbuffer cbPerObject
{
	float4x4 gWorld;
	float4x4 gPrevWorld;
};

cbuffer cbPass
{
	float4x4 gView;
	float4x4 gProj;
	float3 gEyePosW;
};

#endif