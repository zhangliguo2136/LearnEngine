#include "D3DResourceAllocator.h"

TD3DResourceAllocator::TD3DResourceAllocator()
{
}

TD3DResourceAllocator::~TD3DResourceAllocator()
{
}

bool TD3DResourceAllocator::Allocate(TD3DResourceInitInfo& InitInfo, TD3DResource& OutResource)
{
    if (InitInfo.AllocationStrategy == EAllocationStrategy::StandAlone) 
    {
        D3D12_HEAP_PROPERTIES HeapProperties;
        HeapProperties.Type = InitInfo.HeapType;
        HeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        HeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        HeapProperties.CreationNodeMask = 1;
        HeapProperties.VisibleNodeMask = 1;

        D3D12_RESOURCE_DESC ResourcceDesc;
        ResourcceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        ResourcceDesc.Alignment = 0;
        ResourcceDesc.Width = InitInfo.Size;
        ResourcceDesc.Height = 1;
        ResourcceDesc.DepthOrArraySize = 1;
        ResourcceDesc.MipLevels = 1;
        ResourcceDesc.Format = DXGI_FORMAT_UNKNOWN;
        ResourcceDesc.SampleDesc.Count = 1;
        ResourcceDesc.SampleDesc.Quality = 0;
        ResourcceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        ResourcceDesc.Flags = InitInfo.ResourceFlags;

        Microsoft::WRL::ComPtr<ID3D12Resource> Resource;
        D3DDevice->GetD3DDevice()->CreateCommittedResource(
            &HeapProperties, 
            InitInfo.HeapFlags, 
            &ResourcceDesc, 
            InitInfo.InitState,
            nullptr,
            IID_PPV_ARGS(&Resource)
        );

        OutResource.CurrentState = InitInfo.InitState;
        OutResource.D3DResource = Resource;

        return true;
    }

    if (InitInfo.AllocationStrategy == EAllocationStrategy::PlacedResource)
    {
        D3D12_HEAP_PROPERTIES HeapProperties;
        HeapProperties.Type = InitInfo.HeapType;
        HeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        HeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        HeapProperties.CreationNodeMask = 1;
        HeapProperties.VisibleNodeMask = 1;

        D3D12_HEAP_DESC Desc = {};
        Desc.SizeInBytes = DEFAULT_POOL_SIZE;
        Desc.Properties = HeapProperties;
        Desc.Alignment = 0;
        Desc.Flags = InitInfo.HeapFlags;

        ID3D12Heap* Heap = nullptr;
        ThrowIfFailed(D3DDevice->GetD3DDevice()->CreateHeap(&Desc, IID_PPV_ARGS(&Heap)));

        D3D12_RESOURCE_DESC ResourcceDesc;
        ResourcceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        ResourcceDesc.Alignment = 0;
        ResourcceDesc.Width = InitInfo.Size;
        ResourcceDesc.Height = 1;
        ResourcceDesc.DepthOrArraySize = 1;
        ResourcceDesc.MipLevels = 1;
        ResourcceDesc.Format = DXGI_FORMAT_UNKNOWN;
        ResourcceDesc.SampleDesc.Count = 1;
        ResourcceDesc.SampleDesc.Quality = 0;
        ResourcceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        ResourcceDesc.Flags = InitInfo.ResourceFlags;


        // TODO
        //Microsoft::WRL::ComPtr<ID3D12Resource> Resource;
        //D3DDevice->GetD3DDevice()->CreatePlacedResource(
        //    Heap, 
        //    OffsetFromBaseOfHeap,
        //    &ResourcceDesc,
        //    InitInfo.InitState,
        //    nullptr,
        //    IID_PPV_ARGS(&Resource)
        //);

        //OutResource.CurrentState = InitInfo.InitState;
        //OutResource.D3DResource = Resource;
        //OutResource.BackingHeap = Heap;
        //OutResource.OffsetFromBaseOfHeap = OffsetFromBaseOfHeap;

        return true;
    }

    return false;
}
