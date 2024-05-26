#include "D3DRHI.h"
#include "Utils.h"
#include "d3dx12.h"
#include "D3DDevice.h"
#include "D3DCommandContent.h"
#include "D3DViewport.h"
#include "D3DResourceAllocator.h"

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
}

TD3DRHI::~TD3DRHI()
{
}

void TD3DRHI::Initialize(TD3DViewportInfo ViewportInfo)
{
	UINT DXGIFactoryFlags = 0;

#if (defined(DEBUG) || defined(_DEBUG))
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
	Viewport = std::make_unique<TD3DViewport>(this, DXGIFactory, Device.get(), CommandContent.get(), ViewportInfo);
}


void TD3DRHI::Destroy()
{
}

void TD3DRHI::BeginFrame()
{
}

void TD3DRHI::EndFrame()
{
	TempUploadBuffers.clear();
}

void TD3DRHI::UploadBuffer(TD3DResource* DestBuffer, void* Content, uint32_t Size)
{
	auto UploadBuffer = std::make_shared<TD3DResource>();
	TempUploadBuffers.push_back(UploadBuffer);

	TD3DResourceInitInfo UploadBufInitInfo = TD3DResourceInitInfo::Buffer_Upload(Size);
	Device->GetResourceAllocator()->Allocate(UploadBufInitInfo, UploadBuffer.get());

	void* MappedData = nullptr;
	UploadBuffer->D3DResource->Map(0, nullptr, &MappedData);
	memcpy_s(MappedData, Size, Content, Size);
	UploadBuffer->D3DResource->Unmap(0, nullptr);

	TransitionResource(DestBuffer, D3D12_RESOURCE_STATE_COPY_DEST);

	CommandContent->GetCommandList()->CopyBufferRegion(
		DestBuffer->D3DResource.Get(),
		0,
		UploadBuffer->D3DResource.Get(),
		0,
		Size
	);

	TransitionResource(DestBuffer, D3D12_RESOURCE_STATE_COMMON);
}

void TD3DRHI::UploadTexture(TD3DResource* DestTexture, TTextureInfo& SrcTextureInfo)
{
	// 获得纹理图片数据的信息
	D3D12_RESOURCE_DESC DestDesc = DestTexture->D3DResource->GetDesc();
	uint32_t nNumSubresources = 1u;	// 只有一幅图片，即子资源个数为1;
	uint64_t nTextureRowSize = 0u;
	uint32_t nTextureRowNum = 0u;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT nFootprint = {};
	uint64_t nRequiredSize = 0u; //GetRequiredIntermediateSize();

	Device->GetD3DDevice()->GetCopyableFootprints(
		&DestDesc,
		0,
		nNumSubresources,
		0,
		&nFootprint,
		&nTextureRowNum,
		&nTextureRowSize,
		&nRequiredSize
	);

	// 创建上传堆资源
	auto UploadBuffer = std::make_shared<TD3DResource>();
	TempUploadBuffers.push_back(UploadBuffer);

	TD3DResourceInitInfo UploadResInitInfo = TD3DResourceInitInfo::Buffer_Upload(nRequiredSize);
	Device->GetResourceAllocator()->Allocate(UploadResInitInfo, UploadBuffer.get());

	// MemCpy 到上传堆
	BYTE* MappedData = nullptr;
	UploadBuffer->D3DResource->Map(0, NULL, reinterpret_cast<void**>(&MappedData));

	D3D12_SUBRESOURCE_DATA MemCpySrc = {};
	MemCpySrc.pData = SrcTextureInfo.Datas.data();
	MemCpySrc.RowPitch = SrcTextureInfo.RowBytes;
	MemCpySrc.SlicePitch = SrcTextureInfo.TotalBytes;

	D3D12_MEMCPY_DEST MemCpyDest = {};
	MemCpyDest.pData = (BYTE*)MappedData + nFootprint.Offset;
	MemCpyDest.RowPitch = nFootprint.Footprint.RowPitch;
	MemCpyDest.SlicePitch = nFootprint.Footprint.RowPitch * nTextureRowNum;

	for (UINT z = 0; z < nFootprint.Footprint.Depth; ++z)
	{
		auto pDestSlice = static_cast<BYTE*>(MemCpyDest.pData) + MemCpyDest.SlicePitch * z;
		auto pSrcSlice = static_cast<const BYTE*>(MemCpySrc.pData) + MemCpySrc.SlicePitch * LONG_PTR(z);
		for (UINT y = 0; y < nTextureRowNum; ++y)
		{
			memcpy(pDestSlice + MemCpyDest.RowPitch * y,
				pSrcSlice + MemCpySrc.RowPitch * LONG_PTR(y),
				nTextureRowSize);
		}
	}

	UploadBuffer->D3DResource->Unmap(0, NULL);

	this->TransitionResource(DestTexture, D3D12_RESOURCE_STATE_COPY_DEST);

	// 从上传堆复制纹理数据到默认堆
	D3D12_TEXTURE_COPY_LOCATION DstCpyLocation;
	DstCpyLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	DstCpyLocation.pResource = DestTexture->D3DResource.Get();
	DstCpyLocation.SubresourceIndex = 0;

	D3D12_TEXTURE_COPY_LOCATION SrcCpyLocation;
	SrcCpyLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	SrcCpyLocation.pResource = UploadBuffer->D3DResource.Get();
	SrcCpyLocation.PlacedFootprint = nFootprint;
	CommandContent->GetCommandList()->CopyTextureRegion(&DstCpyLocation, 0, 0, 0, &SrcCpyLocation, nullptr);

	this->TransitionResource(DestTexture, D3D12_RESOURCE_STATE_COMMON);
}

void TD3DRHI::TransitionResource(TD3DResource* Resource, D3D12_RESOURCE_STATES StateAfter)
{
	D3D12_RESOURCE_STATES StateBefore = Resource->CurrentState;
	if (StateBefore != StateAfter)
	{
		D3D12_RESOURCE_BARRIER ResourceBarrier = {};
		ResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		ResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		ResourceBarrier.Transition.pResource = Resource->D3DResource.Get();
		ResourceBarrier.Transition.StateAfter = StateAfter;
		ResourceBarrier.Transition.StateBefore = StateBefore;
		ResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		CommandContent->GetCommandList()->ResourceBarrier(1, &ResourceBarrier);
		Resource->CurrentState = StateAfter;
	}
}
