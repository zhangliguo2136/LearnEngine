#include "D3DTexture.h"

TD3DTexture::TD3DTexture()
{
	GpuResource = std::make_shared<TD3DResource>();
}

TD3DTexture::~TD3DTexture()
{
}
