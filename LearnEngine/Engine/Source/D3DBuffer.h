#pragma once
#include <memory>
#include "D3DResource.h"
#include "D3DView.h"

class TD3DBuffer
{
public:
	TD3DBuffer();
	~TD3DBuffer();

public:
	std::shared_ptr<TD3DResource> GpuResource = nullptr;

	std::shared_ptr<TD3DShaderResourceView> SRV = nullptr;
};