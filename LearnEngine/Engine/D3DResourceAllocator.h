#pragma once
#include "D3DResource.h"
#include "D3DDevice.h"

#define DEFAULT_POOL_SIZE (512 * 1024 * 512)

class TD3DResourceAllocator
{
public:
	TD3DResourceAllocator();
	~TD3DResourceAllocator();

	bool Allocate(TD3DResourceInitInfo& InitInfo, TD3DResource& Resource);

private:
	TD3DDevice* D3DDevice = nullptr;
};