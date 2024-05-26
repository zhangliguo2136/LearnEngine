#include "D3DViewport.h"

#include "D3DRHI.h"
#include "D3DDevice.h"
#include "D3DCommandContent.h"
#include "D3DViewAllocator.h"
#include "D3DResourceAllocator.h"

TD3DViewport::TD3DViewport(TD3DRHI* InD3DRHI, Microsoft::WRL::ComPtr<IDXGIFactory4> InFactory, TD3DDevice* InDevice, TD3DCommandContent* InCommandContent, TD3DViewportInfo InViewportInfo)
	:D3DRHI(InD3DRHI), DXGIFactory(InFactory), D3DDevice(InDevice), D3DCommandContent(InCommandContent), ViewportInfo(InViewportInfo)
{
	this->Initialize();
}

TD3DViewport::~TD3DViewport()
{
}

void TD3DViewport::Initialize()
{
	InitSwapChain();
}

void TD3DViewport::InitSwapChain()
{
	DXGISwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC SwapChainDesc;
	SwapChainDesc.BufferDesc.Width = ViewportInfo.Width;
	SwapChainDesc.BufferDesc.Height = ViewportInfo.Height;
	SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	SwapChainDesc.BufferDesc.Format = ViewportInfo.BackBufferFormat;
	SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	SwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	SwapChainDesc.SampleDesc.Count = ViewportInfo.SampleDesc.Count;
	SwapChainDesc.SampleDesc.Quality = ViewportInfo.SampleDesc.Quality;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.BufferCount = SwapChainBufferCount;
	SwapChainDesc.OutputWindow = ViewportInfo.WindowHandle;
	SwapChainDesc.Windowed = true;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue = D3DCommandContent->GetCommandQueue();
	ThrowIfFailed(DXGIFactory->CreateSwapChain(CommandQueue.Get(), &SwapChainDesc, DXGISwapChain.GetAddressOf()));

	this->UpdateViewportSize(ViewportInfo.Width, ViewportInfo.Height);
}

void TD3DViewport::Present()
{
	ThrowIfFailed(DXGISwapChain->Present(0, 0));
	CurrBackBufferIndex = (CurrBackBufferIndex + 1) % SwapChainBufferCount;
}

void TD3DViewport::UpdateViewportSize(uint32_t Width, uint32_t Height)
{
	ViewportInfo.Width = Width;
	ViewportInfo.Height = Height;

	D3DCommandContent->ResetCommandList();

	for (UINT i = 0; i < SwapChainBufferCount; ++i)
	{
		RenderTargetTextures[i].reset();
	}
	DepthStencilTexture.reset();

	ThrowIfFailed(DXGISwapChain->ResizeBuffers(SwapChainBufferCount, ViewportInfo.Width, ViewportInfo.Height, ViewportInfo.BackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
	CurrBackBufferIndex = 0;


	auto ViewAllocator = D3DDevice->GetViewAllocator();
	auto ResourceAllocator = D3DDevice->GetResourceAllocator();

	//Create RenderTargetTextures
	for (UINT i = 0; i < SwapChainBufferCount; ++i)
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> SwapChainBuffer = nullptr;
		ThrowIfFailed(DXGISwapChain->GetBuffer(i, IID_PPV_ARGS(SwapChainBuffer.GetAddressOf())));

		TD3DTextureRef TextureRef = std::make_shared<TD3DTexture>();
		TextureRef->GpuResource = std::make_shared<TD3DResource>(SwapChainBuffer, D3D12_RESOURCE_STATE_PRESENT);
		TextureRef->RTV = std::make_shared<TD3DRenderTargetView>(D3DDevice, TextureRef->GpuResource.get());
		RenderTargetTextures[i] = TextureRef;

		D3D12_RESOURCE_DESC BackBufferDesc = SwapChainBuffer->GetDesc();

		TD3DViewInitInfo RTViewInfo = TD3DViewInitInfo::RTView_Texture2D(TextureRef->GpuResource.get(), BackBufferDesc.Format);
		ViewAllocator->Allocate(TextureRef->RTV.get(), RTViewInfo);
	}

	// Create DepthStencilTexture
	{
		TD3DTextureRef TextureRef = std::make_shared<TD3DTexture>();
		TextureRef->DSV = std::make_shared<TD3DDepthStencilView>(D3DDevice, TextureRef->GpuResource.get());
		TextureRef->SRV = std::make_shared<TD3DShaderResourceView>(D3DDevice, TextureRef->GpuResource.get());
		DepthStencilTexture = TextureRef;

		// Create Resource Of Texture
		TD3DResourceInitInfo ResInitInfo = TD3DResourceInitInfo::Texture2D(ViewportInfo.Width, ViewportInfo.Height, DXGI_FORMAT_R24G8_TYPELESS);
		ResInitInfo.ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		ResInitInfo.InitState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		ResourceAllocator->Allocate(ResInitInfo, TextureRef->GpuResource.get());

		// Create DSV Of Texture
		TD3DViewInitInfo DSViewInfo = TD3DViewInitInfo::DSView_Texture2D(TextureRef->GpuResource.get(), DXGI_FORMAT_D24_UNORM_S8_UINT);
		ViewAllocator->Allocate(TextureRef->DSV.get(), DSViewInfo);

		// Create SRV Of Texture
		TD3DViewInitInfo SRViewInfo = TD3DViewInitInfo::SRView_Texture2D(TextureRef->GpuResource.get(), DXGI_FORMAT_R24_UNORM_X8_TYPELESS);
		ViewAllocator->Allocate(TextureRef->SRV.get(), SRViewInfo);
	}

	D3DCommandContent->ExecuteCommandList();
	D3DCommandContent->FlushCommandQueue();
}

void TD3DViewport::GetD3DViewport(D3D12_VIEWPORT& OutD3DViewPort, D3D12_RECT& OutD3DRect)
{
	OutD3DViewPort.TopLeftX = 0;
	OutD3DViewPort.TopLeftY = 0;
	OutD3DViewPort.Width = static_cast<float>(ViewportInfo.Width);
	OutD3DViewPort.Height = static_cast<float>(ViewportInfo.Height);
	OutD3DViewPort.MinDepth = 0.0f;
	OutD3DViewPort.MaxDepth = 1.0f;

	OutD3DRect = { 0, 0, (int)ViewportInfo.Width, (int)ViewportInfo.Height };
}

TD3DDepthStencilView* TD3DViewport::GetDepthStencilView()
{
	return DepthStencilTexture->DSV.get();
}

TD3DTextureRef TD3DViewport::GetCurrentBackRT()
{
	return RenderTargetTextures[CurrBackBufferIndex];
}
