#pragma once

#include <memory>
#include <string>

#include "D3DResource.h"
#include "D3DView.h"

class TD3DTexture
{
public:
	TD3DTexture(std::string InName = "");
	~TD3DTexture();

public:

	// 描述符描述的是这个资源的用途
	std::shared_ptr<TD3DShaderResourceView> SRV = nullptr;
	std::shared_ptr<TD3DUnorderedAccessView> UAV = nullptr;
	std::shared_ptr<TD3DRenderTargetView> RTV = nullptr;
	std::shared_ptr<TD3DDepthStencilView> DSV = nullptr;

public:
	std::shared_ptr<TD3DResource> GpuResource = nullptr;
};

typedef std::shared_ptr<TD3DTexture> TD3DTextureRef;
