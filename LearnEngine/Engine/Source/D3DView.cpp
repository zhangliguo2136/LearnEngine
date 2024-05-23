#include "D3DView.h"
#include "D3DResource.h"

TD3DView::TD3DView(TD3DDevice* InDevice, TD3DResource* InResource, D3D12_DESCRIPTOR_HEAP_TYPE InHeapType)
	:Device(InDevice), Resource(InResource), HeapType(InHeapType)
{
	DescriptorAllocator = Device->GetDescriptorAllocator(InHeapType);
	
	if (DescriptorAllocator)
	{
		Descriptor = DescriptorAllocator->AllocateDescriptor();
	}
}

TD3DView::~TD3DView()
{
	if (DescriptorAllocator)
	{
		DescriptorAllocator->FreeDescriptor(Descriptor);
	}
}

TD3DShaderResourceView::TD3DShaderResourceView(TD3DDevice* InDevice, TD3DResource* InResource)
	:TD3DView(InDevice, InResource, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
{
}

TD3DShaderResourceView::~TD3DShaderResourceView()
{
}

TD3DUnorderedAccessView::TD3DUnorderedAccessView(TD3DDevice* InDevice, TD3DResource* InResource)
	:TD3DView(InDevice, InResource, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
{
}

TD3DUnorderedAccessView::~TD3DUnorderedAccessView()
{
}

TD3DRenderTargetView::TD3DRenderTargetView(TD3DDevice* InDevice, TD3DResource* InResource)
	:TD3DView(InDevice, InResource, D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
{
}

TD3DRenderTargetView::~TD3DRenderTargetView()
{
}

TD3DDepthStencilView::TD3DDepthStencilView(TD3DDevice* InDevice, TD3DResource* InResource)
	:TD3DView(InDevice, InResource, D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
{
}

TD3DDepthStencilView::~TD3DDepthStencilView()
{
}

TD3DViewInitInfo TD3DViewInitInfo::SRView_Texture2D(TD3DResource* InResource, DXGI_FORMAT SRVFormat)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC SRView = {};
	SRView.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SRView.Format = SRVFormat;
	SRView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	SRView.Texture2D.MostDetailedMip = 0;
	SRView.Texture2D.MipLevels = 1;
	SRView.Texture2D.PlaneSlice = 0;

	TD3DViewInitInfo Info;
	Info.SRView = SRView;
	Info.Resource = InResource;

	return Info;
}

TD3DViewInitInfo TD3DViewInitInfo::DSView_Texture2D(TD3DResource* InResource, DXGI_FORMAT DSVFormat)
{
	D3D12_DEPTH_STENCIL_VIEW_DESC DSView = {};
	DSView.Flags = D3D12_DSV_FLAG_NONE;
	DSView.Format = DSVFormat;
	DSView.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	DSView.Texture2D.MipSlice = 0;

	TD3DViewInitInfo Info;
	Info.DSView = DSView;
	Info.Resource = InResource;

	return Info;
}

TD3DViewInitInfo TD3DViewInitInfo::RTView_Texture2D(TD3DResource* InResource, DXGI_FORMAT RTVFormat)
{

	D3D12_RENDER_TARGET_VIEW_DESC RTView = {};
	RTView.Format = RTVFormat;
	RTView.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	RTView.Texture2D.MipSlice = 0;
	RTView.Texture2D.PlaneSlice = 0;

	TD3DViewInitInfo Info;
	Info.RTView = RTView;
	Info.Resource = InResource;

	return Info;
}

TD3DViewInitInfo TD3DViewInitInfo::SRView_Buffer(TD3DResource* InResource, uint32_t ElementCount, uint32_t ElementSize)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC SRView = {};
	SRView.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SRView.Format = DXGI_FORMAT_UNKNOWN;
	SRView.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	SRView.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	SRView.Buffer.StructureByteStride = ElementSize;
	SRView.Buffer.NumElements = ElementCount;
	SRView.Buffer.FirstElement = 0;

	TD3DViewInitInfo Info;
	Info.SRView = SRView;
	Info.Resource = InResource;

	return Info;
}
