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
	CurrBackBufferIndex = (CurrBackBufferIndex - 1) % SwapChainBufferCount;
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
		RenderTargetTextures[i] = TextureRef;

		D3D12_RESOURCE_DESC BackBufferDesc = SwapChainBuffer->GetDesc();
		// Create RTV Of Texture
		D3D12_RENDER_TARGET_VIEW_DESC RtvDesc = {};
		RtvDesc.Format = BackBufferDesc.Format;
		RtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		RtvDesc.Texture2D.MipSlice = 0;
		RtvDesc.Texture2D.PlaneSlice = 0;

		auto RTView = std::make_shared<TD3DRenderTargetView>(D3DDevice, TextureRef->GpuResource.get());
		ViewAllocator->CreateRenderTargetView(TextureRef->GpuResource.get(), RTView.get(), RtvDesc);
		TextureRef->RTV = RTView;
	}

	// Create DepthStencilTexture
	{
		TD3DTextureRef TextureRef = std::make_shared<TD3DTexture>();

		// Create Resource Of Texture
		TD3DResourceInitInfo ResInitInfo = TD3DResourceInitInfo::Texture2D(ViewportInfo.Width, ViewportInfo.Height, DXGI_FORMAT_R24G8_TYPELESS);
		ResInitInfo.ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		ResInitInfo.ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		ResInitInfo.InitState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		ResourceAllocator->Allocate(ResInitInfo, TextureRef->GpuResource.get());

		// Create DSV Of Texture
		D3D12_DEPTH_STENCIL_VIEW_DESC DSVDesc = {};
		DSVDesc.Flags = D3D12_DSV_FLAG_NONE;
		DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		DSVDesc.Texture2D.MipSlice = 0;
		DSVDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		auto DSView = std::make_shared<TD3DDepthStencilView>(D3DDevice, TextureRef->GpuResource.get());
		ViewAllocator->CreateDepthStencilView(TextureRef->GpuResource.get(), DSView.get(), DSVDesc);
		TextureRef->DSV = DSView;

		// Create SRV Of Texture
		D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
		SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SrvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		SrvDesc.Texture2D.MostDetailedMip = 0;
		SrvDesc.Texture2D.MipLevels = 1;
		auto SRView = std::make_shared<TD3DShaderResourceView>(D3DDevice, TextureRef->GpuResource.get());
		ViewAllocator->CreateShaderResourceView(TextureRef->GpuResource.get(), SRView.get(), SrvDesc);
		TextureRef->SRV = SRView;

		DepthStencilTexture = TextureRef;
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
