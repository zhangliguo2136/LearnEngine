#pragma once
#include <wrl/module.h>
#include <dxgi1_4.h>
#include <stdint.h>

#include "D3DTexture.h"

struct TD3DViewportInfo
{
	DXGI_FORMAT BackBufferFormat;
	DXGI_FORMAT DepthStencilFormat;

	//MSAA
	DXGI_SAMPLE_DESC SampleDesc = {1, 0};

	UINT Width = 0;
	UINT Height = 0;

	HWND WindowHandle;
};

class TD3DRHI;
class TD3DDevice;
class TD3DCommandContent;

class TD3DViewport
{
public:
	TD3DViewport(TD3DRHI* InRHI, Microsoft::WRL::ComPtr<IDXGIFactory4> InFactory, TD3DDevice* InDevice, TD3DCommandContent* InCommandContent, TD3DViewportInfo ViewportInfo);
	~TD3DViewport();

private:
	void Initialize();
	void InitSwapChain();

public:
	void Present();

public:
	TD3DViewportInfo GetViewportInfo() const { return ViewportInfo; };
	void UpdateViewportSize(uint32_t Width, uint32_t Height);
	void GetD3DViewport(D3D12_VIEWPORT& OutD3DViewPort, D3D12_RECT& OutD3DRect);

	TD3DDepthStencilView* GetDepthStencilView();
	TD3DTextureRef GetCurrentBackRT();
private:
	static const int SwapChainBufferCount = 2;
	int CurrBackBufferIndex = 0;

	TD3DViewportInfo ViewportInfo;

	Microsoft::WRL::ComPtr<IDXGIFactory4> DXGIFactory = nullptr;
	Microsoft::WRL::ComPtr<IDXGISwapChain> DXGISwapChain = nullptr;

	TD3DRHI* D3DRHI = nullptr;
	TD3DDevice* D3DDevice = nullptr;
	TD3DCommandContent* D3DCommandContent = nullptr;

private:
	TD3DTextureRef RenderTargetTextures[SwapChainBufferCount];
	TD3DTextureRef DepthStencilTexture = nullptr;
};