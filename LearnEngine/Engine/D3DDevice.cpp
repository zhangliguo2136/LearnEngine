#include "D3DDevice.h"

#include "D3DRHI.h"

TD3DDevice::TD3DDevice(TD3DRHI* InD3DRHI, Microsoft::WRL::ComPtr<IDXGIFactory4> Factory)
	:D3DRHI(InD3DRHI), DXGIFactory(Factory)
{
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
