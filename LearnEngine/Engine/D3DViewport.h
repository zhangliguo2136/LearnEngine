#pragma once
#include <wrl/module.h>
#include <dxgi1_4.h>
#include <stdint.h>

#include "D3DTexture.h"

struct TD3DViewportInfo
{
	DXGI_FORMAT BackBufferFormat;
	DXGI_FORMAT DepthStencilFormat;
	bool bEnable4X_MSAA = false;
	UINT QualityOf4X_MSAA = 0;

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
	TD3DViewport(TD3DRHI* InRHI, Microsoft::WRL::ComPtr<IDXGIFactory4> InFactory, TD3DDevice* InDevice, TD3DCommandContent* InCommandContent);
	~TD3DViewport();

private:
	void Initialize();
	void InitSwapChain();

public:
	void Present();

public:
	TD3DViewportInfo GetViewportInfo() const { return ViewportInfo; };
	void UpdateViewportSize(uint32_t Width, uint32_t Height);

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