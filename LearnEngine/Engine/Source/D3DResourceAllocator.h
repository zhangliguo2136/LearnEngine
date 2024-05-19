#pragma once
#include "D3DResource.h"
#include "D3DDevice.h"

#define DEFAULT_POOL_SIZE (512 * 1024 * 512)

class TD3DDevice;
class TD3DResourceAllocator
{
public:
	TD3DResourceAllocator(TD3DDevice* InDevice);
	~TD3DResourceAllocator();

	bool Allocate(const TD3DResourceInitInfo& InitInfo, TD3DResource* Resource);

private:
	TD3DDevice* Device = nullptr;
};