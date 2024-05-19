#include "D3DViewport.h"

#include "D3DRHI.h"
#include "D3DDevice.h"
#include "D3DCommandContent.h"

TD3DViewport::TD3DViewport(TD3DRHI* InD3DRHI, Microsoft::WRL::ComPtr<IDXGIFactory4> InFactory, TD3DDevice* InDevice, TD3DCommandContent* InCommandContent)
	:D3DRHI(InD3DRHI), DXGIFactory(InFactory), D3DDevice(InDevice), D3DCommandContent(InCommandContent)
{
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
	SwapChainDesc.SampleDesc.Count = ViewportInfo.bEnable4X_MSAA ? 4 : 1;
	SwapChainDesc.SampleDesc.Quality = ViewportInfo.bEnable4X_MSAA ? (ViewportInfo.QualityOf4X_MSAA - 1) : 0;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.BufferCount = SwapChainBufferCount;
	SwapChainDesc.OutputWindow = ViewportInfo.WindowHandle;
	SwapChainDesc.Windowed = true;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue = D3DCommandContent->GetCommandQueue();
	ThrowIfFailed(DXGIFactory->CreateSwapChain(CommandQueue.Get(), &SwapChainDesc, DXGISwapChain.GetAddressOf()));
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

	D3DCommandContent->FlushCommandQueue();
	D3DCommandContent->ResetCommandList();

	for (UINT i = 0; i < SwapChainBufferCount; ++i)
	{
		RenderTargetTextures[i].reset();
	}
	DepthStencilTexture.reset();

	ThrowIfFailed(DXGISwapChain->ResizeBuffers(SwapChainBufferCount, ViewportInfo.Width, ViewportInfo.Height, ViewportInfo.BackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
	CurrBackBufferIndex = 0;


	//Create RenderTargetTextures
	for (UINT i = 0; i < SwapChainBufferCount; ++i)
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> SwapChainBuffer = nullptr;
		ThrowIfFailed(DXGISwapChain->GetBuffer(i, IID_PPV_ARGS(SwapChainBuffer.GetAddressOf())));

		TD3DTextureRef TextureRef = std::make_shared<TD3DTexture>();
		TextureRef->GpuResource = TD3DResource(SwapChainBuffer, D3D12_RESOURCE_STATE_PRESENT);
		RenderTargetTextures[i] = TextureRef;

		D3D12_RESOURCE_DESC BackBufferDesc = SwapChainBuffer->GetDesc();
		// Create RTV Of Texture
		D3D12_RENDER_TARGET_VIEW_DESC RtvDesc = {};
		RtvDesc.Format = BackBufferDesc.Format;
		RtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		RtvDesc.Texture2D.MipSlice = 0;
		RtvDesc.Texture2D.PlaneSlice = 0;

		auto RTV = std::make_unique<TD3DRenderTargetView>();
		D3DDevice->GetD3DDevice()->CreateRenderTargetView(SwapChainBuffer.Get(), &RtvDesc, RTV->GetCpuDescriptorHandle());
		TextureRef->RTV = std::move(RTV);
	}

	// Create DepthStencilTexture
	{
		// Create Resource Of Texture
		D3D12_RESOURCE_STATES ResourceState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
		D3D12_RESOURCE_DESC TexDesc;
		ZeroMemory(&TexDesc, sizeof(D3D12_RESOURCE_DESC));
		TexDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		TexDesc.Alignment = 0;
		TexDesc.Width = ViewportInfo.Width;
		TexDesc.Height = ViewportInfo.Height;
		TexDesc.DepthOrArraySize = 1;
		TexDesc.MipLevels = 1;
		TexDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		TexDesc.SampleDesc.Count = 1;
		TexDesc.SampleDesc.Quality = 0;
		TexDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

		// Create DSV Of Texture
		D3D12_DEPTH_STENCIL_VIEW_DESC DSVDesc = {};
		DSVDesc.Flags = D3D12_DSV_FLAG_NONE;
		DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		DSVDesc.Texture2D.MipSlice = 0;
		DSVDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

		// Create SRV Of Texture
		D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
		SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SrvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		SrvDesc.Texture2D.MostDetailedMip = 0;
		SrvDesc.Texture2D.MipLevels = 1;
	}

	D3DCommandContent->ExecuteCommands();
	D3DCommandContent->FlushCommandQueue();
}
