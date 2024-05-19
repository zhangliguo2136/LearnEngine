#include "D3DViewAllocator.h"
#include "D3DDevice.h"
#include "D3DResource.h"
#include "D3DView.h"

TD3DViewAllocator::TD3DViewAllocator(TD3DDevice* InDevice)
	:Device(InDevice)
{
}

TD3DViewAllocator::~TD3DViewAllocator()
{
}

void TD3DViewAllocator::CreateRenderTargetView(TD3DResource* InResource, TD3DRenderTargetView* InRenderTargetView, D3D12_RENDER_TARGET_VIEW_DESC RTVDesc)
{
	Device->GetD3DDevice()->CreateRenderTargetView(InResource->D3DResource.Get(), &RTVDesc, InRenderTargetView->GetCpuDescriptorHandle());
}

void TD3DViewAllocator::CreateDepthStencilView(TD3DResource* InResource, TD3DDepthStencilView* InDepthStencilView, D3D12_DEPTH_STENCIL_VIEW_DESC DSVDesc)
{
	Device->GetD3DDevice()->CreateDepthStencilView(InResource->D3DResource.Get(), &DSVDesc, InDepthStencilView->GetCpuDescriptorHandle());
}

void TD3DViewAllocator::CreateShaderResourceView(TD3DResource* InResource, TD3DShaderResourceView* InShaderResourceView, D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc)
{
	Device->GetD3DDevice()->CreateShaderResourceView(InResource->D3DResource.Get(), &SRVDesc, InShaderResourceView->GetCpuDescriptorHandle());
}
