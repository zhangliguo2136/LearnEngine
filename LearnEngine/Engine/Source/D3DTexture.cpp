#include "D3DTexture.h"

TD3DTexture::TD3DTexture(std::string InName)
{
	GpuResource = std::make_shared<TD3DResource>(InName);
}

TD3DTexture::~TD3DTexture()
{
}
