#include "D3DRHI.h"
#include "Utils.h"
#include "d3dx12.h"
#include "D3DDevice.h"
#include "D3DCommandContent.h"
#include "D3DViewport.h"

#include <intsafe.h>
#include <wrl/module.h>
#include <d3d12sdklayers.h>
#include <winerror.h>
#include <dxgidebug.h>
#include <dxgi1_3.h>
#include <vector>
#include <assert.h>

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")


TD3DRHI::TD3DRHI()
{
	Initialize();
}

TD3DRHI::~TD3DRHI()
{
}

void TD3DRHI::Initialize()
{
	UINT DXGIFactoryFlags = 0;

#if (defined(DEBUG) || defined(_DEBUG)) && InstalledDebugLayers
	{
		Microsoft::WRL::ComPtr<ID3D12Debug> DebugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(DebugController.GetAddressOf())));
		DebugController->EnableDebugLayer();
	}
#endif


	Microsoft::WRL::ComPtr<IDXGIInfoQueue> DXGIInfoQueue;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(DXGIInfoQueue.GetAddressOf()))))
	{
		DXGIFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
		DXGIInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
		DXGIInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
	}

	// Create DXGIFactory
	Microsoft::WRL::ComPtr<IDXGIFactory4> DXGIFactory = nullptr;
	ThrowIfFailed(CreateDXGIFactory2(DXGIFactoryFlags, IID_PPV_ARGS(DXGIFactory.GetAddressOf())));

	// Create D3DDevice;
	Device = std::make_unique<TD3DDevice>(this, DXGIFactory);

	// Create CommandContent
	CommandContent = std::make_unique<TD3DCommandContent>(this, Device.get());

	// Create D3DSwapChain||Viewport
	Viewport = std::make_unique<TD3DViewport>(this, DXGIFactory, Device.get(), CommandContent.get());
}

void TD3DRHI::Destroy()
{
}
