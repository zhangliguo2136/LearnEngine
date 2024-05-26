#ifndef __SHADER_UTILS__
#define __SHADER_UTILS__

float2 NDCToUV(float4 NDCPos)
{
	return float2(0.5 + 0.5 * NDCPos.x, 0.5 - 0.5 * NDCPos.y);
}

// Transforms a normal map sample to world space.
float3 NormalSampleToWorldSpace(float3 NormalMapSample, float3 UnitNormalW, float3 TangentW)
{
	// Uncompress each component from [0,1] to [-1,1].
	float3 NormalT = 2.0f * NormalMapSample - 1.0f;

	// Build orthonormal basis.
	float3 N = UnitNormalW;
	float3 T = normalize(TangentW - dot(TangentW, N) * N);
	float3 B = cross(N, T);

	float3x3 TBN = float3x3(T, B, N);

	// Transform from tangent space to world space.
	float3 BumpedNormalW = mul(NormalT, TBN);

	return BumpedNormalW;
}

#endif