#ifndef __SHADER_MATERIAL__
#define __SHADER_MATERIAL__

struct Material
{
    uint ShadingModel;
    float3 EmissiveColor;
};
ConstantBuffer<Material> cbMaterial;

#endif
