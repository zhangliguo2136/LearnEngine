static const uint LightCount = 0;
const static float PI = 3.14159265359;

struct LightParameters
{
    float3 Color;
    float Intensity;

    uint LightType;
    float3 Direction;
};

//D:法线分布函数 (Trowbridge-Reitz)
float D_DistributionGGX(float3 NdotH, float Roughness)
{
    float a2 = Roughness * Roughness;
    
    float denom = (NdotH * NdotH * (a2 - 1) + 1);
    denom = PI * denom * denom;

    return a2 / denom;
}

//F:反射函数：Schlick 的 Fresnel方法
float3 F_SchlickFresnel(float3 F0, float3 HdotL, float3 VdotH)
{
    // 标准公式
    return F0 + (1 - F0) * pow((1 - HdotL), 5);

    // UE magic
    return F0 + (1 - F0) * exp((-5.55473 * VdotH - 6.98316) * VdotH);

}

// G
float G_SchlickGGX(float3 NdotV, float Roughness)
{
    float k = pow(Roughness + 1, 2) / 8.0f;
    
    return NdotV / (NdotV * (1 - k) + k);
}

// BRDF
static const float F0_DIELECTRIC = 0.04f;
float3 DefaultBRDF(float3 LightDir, float3 ViewDir, float3 Normal, float3 BaseColor, float Roughness, float Metallic)
{
    float halfVec = normalize(ViewDir + LightDir);
    float NdotL = saturate(dot(Normal, LightDir));
    float NdotV = saturate(dot(Normal, ViewDir));
    float VdotH = saturate(dot(ViewDir, halfVec));
    float NdotH = saturate(dot(Normal, halfVec));
    float HdotL = saturate(dot(halfVec, LightDir));
    
    //镜面反射部分
    //反射率（反射率基于F0和base color之间）
    float3 F0 = lerp(F0_DIELECTRIC.rrr, BaseColor.rgb, Metallic);
    // 法线分布函数：决定了specular 高光的大小亮度和形状
    float D = D_DistributionGGX(NdotH, Roughness);
    // 反射函数：计算光学上的反射比率
    float3 F = F_SchlickFresnel(F0, HdotL, VdotH);
    // 阴影遮罩函数（几何函数）：未被shadow或mask的比例
    float G = G_SchlickGGX(NdotV, Roughness) * G_SchlickGGX(NdotL, Roughness);
    // 校正因子:用来校正从microfacet的局部空间转到整体表面的数量差异(避免除零错误)
    float Factor = max(4 * NdotL * NdotV, 0.001);
    
    float3 SpecularBRDF = (D * G * F) / Factor;
    
    //漫反射部分
    float3 DiffuseColor = (1.0 - Metallic) * BaseColor * (1 - F);
    float3 DiffuseBRDF = DiffuseColor * (1 / PI);
    
    return DiffuseBRDF + SpecularBRDF;

}

float3 DirectLighting(float3 Radiance, float3 LightDir, float3 ViewDir, float3 Normal, float3 BaseColor, float Roughness, float Metallic)
{
    float3 BRDF = DefaultBRDF(LightDir, ViewDir, Normal, BaseColor, Roughness, Metallic);
    float NdotL = saturate(dot(Normal, LightDir));
    
    return Radiance * BRDF * NdotL;
}