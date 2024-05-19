#pragma once
#include <d3d12.h>
#include <stdint.h>
#include <wrl/module.h>
#include <dxgi1_4.h>
#include <memory>

#include "Utils.h"

class TD3DRHI;
class TD3DViewAllocator;
class TD3DResourceAllocator;
class TD3DDescriptorAllocator;

class TD3DDevice
{
public:
	TD3DDevice(TD3DRHI*, Microsoft::WRL::ComPtr<IDXGIFactory4>);
	~TD3DDevice();

	void Initialize();

	ID3D12Device* GetD3DDevice() 
	{
		return D3DDevice.Get();
	}

	TD3DViewAllocator* GetViewAllocator()
	{
		return ViewAllocator;
	}

	TD3DResourceAllocator* GetResourceAllocator()
	{
		return ResourceAllocator;
	}

	TD3DDescriptorAllocator* GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE InHeapType);

private:
	Microsoft::WRL::ComPtr<ID3D12Device> D3DDevice = nullptr;
	Microsoft::WRL::ComPtr<IDXGIFactory4> DXGIFactory = nullptr;

	TD3DRHI* D3DRHI = nullptr;
	TD3DViewAllocator* ViewAllocator = nullptr;
	TD3DResourceAllocator* ResourceAllocator = nullptr;

	std::unique_ptr<TD3DDescriptorAllocator> RTVDescriptorAllocator = nullptr;
	std::unique_ptr<TD3DDescriptorAllocator> SRVDescriptorAllocator = nullptr;
	std::unique_ptr<TD3DDescriptorAllocator> DSVDescriptorAllocator = nullptr;
};