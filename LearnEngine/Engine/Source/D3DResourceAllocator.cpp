#include "D3DResourceAllocator.h"

TD3DResourceAllocator::TD3DResourceAllocator(TD3DDevice* InDevice)
    :Device(InDevice)
{
}

TD3DResourceAllocator::~TD3DResourceAllocator()
{
}

bool TD3DResourceAllocator::Allocate(const TD3DResourceInitInfo& InitInfo, TD3DResource* OutResource)
{
    if (InitInfo.AllocationStrategy == EAllocationStrategy::StandAlone) 
    {

        D3D12_CLEAR_VALUE* ClearValuePtr = nullptr;
        if (InitInfo.ResourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER)
        {
            if (InitInfo.ResourceDesc.Flags == D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
            {
                D3D12_CLEAR_VALUE ClearValue;
                ClearValue.Format = InitInfo.ResourceDesc.Format;
                ClearValue.Color[0] = 0.0f;
                ClearValue.Color[1] = 0.0f;
                ClearValue.Color[2] = 0.0f;
                ClearValue.Color[3] = 1.0f;

                ClearValuePtr = &ClearValue;
            }
            else if (InitInfo.ResourceDesc.Flags == D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
            {
                D3D12_CLEAR_VALUE ClearValue;
                ClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
                ClearValue.DepthStencil.Depth = 1.0f;;
                ClearValue.DepthStencil.Stencil = 0;
     
                ClearValuePtr = &ClearValue;
            }
        }

        ThrowIfFailed(Device->GetD3DDevice()->CreateCommittedResource(
            &InitInfo.HeapProperties,
            InitInfo.HeapFlags,
            &InitInfo.ResourceDesc,
            InitInfo.InitState,
            ClearValuePtr,
            IID_PPV_ARGS(&OutResource->D3DResource)
        ));


        OutResource->CurrentState = InitInfo.InitState;
        return true;
    }

    if (InitInfo.AllocationStrategy == EAllocationStrategy::PlacedResource)
    {
        //D3D12_HEAP_PROPERTIES HeapProperties;
        //HeapProperties.Type = InitInfo.HeapType;
        //HeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        //HeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        //HeapProperties.CreationNodeMask = 1;
        //HeapProperties.VisibleNodeMask = 1;

        //D3D12_HEAP_DESC Desc = {};
        //Desc.SizeInBytes = DEFAULT_POOL_SIZE;
        //Desc.Properties = InitInfo.HeapProperties;
        //Desc.Alignment = 0;
        //Desc.Flags = InitInfo.HeapFlags;

        //ID3D12Heap* Heap = nullptr;
        //ThrowIfFailed(D3DDevice->GetD3DDevice()->CreateHeap(&InitInfo.HeapDesc, IID_PPV_ARGS(&Heap)));

        //D3D12_RESOURCE_DESC ResourcceDesc;
        //ResourcceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        //ResourcceDesc.Alignment = 0;
        //ResourcceDesc.Width = InitInfo.Size;
        //ResourcceDesc.Height = 1;
        //ResourcceDesc.DepthOrArraySize = 1;
        //ResourcceDesc.MipLevels = 1;
        //ResourcceDesc.Format = DXGI_FORMAT_UNKNOWN;
        //ResourcceDesc.SampleDesc.Count = 1;
        //ResourcceDesc.SampleDesc.Quality = 0;
        //ResourcceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        //ResourcceDesc.Flags = InitInfo.ResourceFlags;


        // TODO
        //Microsoft::WRL::ComPtr<ID3D12Resource> Resource;
        //Device->GetD3DDevice()->CreatePlacedResource(
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
