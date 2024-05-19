#include "D3DDevice.h"

#include "D3DRHI.h"
#include "D3DViewAllocator.h"
#include "D3DResourceAllocator.h"
#include "D3DDescriptorAllocator.h"

TD3DDevice::TD3DDevice(TD3DRHI* InD3DRHI, Microsoft::WRL::ComPtr<IDXGIFactory4> Factory)
	:D3DRHI(InD3DRHI), DXGIFactory(Factory)
{
	this->Initialize();

	ViewAllocator = new TD3DViewAllocator(this);
	ResourceAllocator = new TD3DResourceAllocator(this);

	//Create Descriptor allocator
	RTVDescriptorAllocator = std::make_unique<TD3DDescriptorAllocator>(this, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 200);
	DSVDescriptorAllocator = std::make_unique<TD3DDescriptorAllocator>(this, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 200);
	SRVDescriptorAllocator = std::make_unique<TD3DDescriptorAllocator>(this, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 200);
}

TD3DDevice::~TD3DDevice()
{
}

void TD3DDevice::Initialize()
{
	// Try to create hardware device.
	HRESULT HardwareResult = D3D12CreateDevice(nullptr/*default adapter*/, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&D3DDevice));

	// Fallback to WARP device.
	if (FAILED(HardwareResult))
	{
		Microsoft::WRL::ComPtr<IDXGIAdapter> WarpAdapter;
		ThrowIfFailed(DXGIFactory->EnumWarpAdapter(IID_PPV_ARGS(&WarpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(WarpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&D3DDevice)));
	}
}

TD3DDescriptorAllocator* TD3DDevice::GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE InHeapType)
{
	switch (InHeapType)
	{
	case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
		return SRVDescriptorAllocator.get();
		break;
	case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
		return RTVDescriptorAllocator.get();
		break;
	case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
		return DSVDescriptorAllocator.get();
		break;
	default:
		return nullptr;
	}
}
