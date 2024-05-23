#ifndef __SHADER_SAMPLER__
#define __SHADER_SAMPLER__

SamplerState SamplerPointWrap   :register(s0);
SamplerState SamplerPointClamp  :register(s1);
SamplerState SamplerLinearWrap  :register(s2);
SamplerState SamplerLinearClamp :register(s3);
SamplerState SamplerAnisotropicWrap     :register(s4);
SamplerState SamplerAnisotropicClamp    :register(s5);

#endif
