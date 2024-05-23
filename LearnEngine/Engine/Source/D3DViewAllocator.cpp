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

void TD3DViewAllocator::Allocate(TD3DRenderTargetView* InRenderTarget, TD3DViewInitInfo InitInfo)
{
	Device->GetD3DDevice()->CreateRenderTargetView(InitInfo.Resource->D3DResource.Get(), &InitInfo.RTView, InRenderTarget->GetCpuDescriptorHandle());
}

void TD3DViewAllocator::Allocate(TD3DDepthStencilView* InDepthStencilView, TD3DViewInitInfo InitInfo)
{
	Device->GetD3DDevice()->CreateDepthStencilView(InitInfo.Resource->D3DResource.Get(), &InitInfo.DSView, InDepthStencilView->GetCpuDescriptorHandle());
}

void TD3DViewAllocator::Allocate(TD3DShaderResourceView* InShaderResourceView, TD3DViewInitInfo InitInfo)
{
	Device->GetD3DDevice()->CreateShaderResourceView(InitInfo.Resource->D3DResource.Get(), &InitInfo.SRView, InShaderResourceView->GetCpuDescriptorHandle());
}
