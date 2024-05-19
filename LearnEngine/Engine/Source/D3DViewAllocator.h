#pragma once
#include "d3dx12.h"

class TD3DDevice;
class TD3DResource;
class TD3DRenderTargetView;
class TD3DDepthStencilView;
class TD3DShaderResourceView;
class TD3DViewAllocator
{
public:
	TD3DViewAllocator(TD3DDevice* InDevice);
	~TD3DViewAllocator();

	void CreateRenderTargetView(TD3DResource* InResource, TD3DRenderTargetView* InRenderTargetView, D3D12_RENDER_TARGET_VIEW_DESC RTVDesc);
	void CreateDepthStencilView(TD3DResource* InResource, TD3DDepthStencilView* InDepthStencilView, D3D12_DEPTH_STENCIL_VIEW_DESC DSVDesc);
	void CreateShaderResourceView(TD3DResource* InResource, TD3DShaderResourceView* InShaderResourceView, D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc);

private:
	TD3DDevice* Device = nullptr;
};
