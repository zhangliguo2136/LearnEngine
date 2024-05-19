#include "D3DResource.h"

TD3DResource::TD3DResource()
{
}

TD3DResource::TD3DResource(Microsoft::WRL::ComPtr<ID3D12Resource> InD3DResource, D3D12_RESOURCE_STATES InState)
	:D3DResource(InD3DResource), CurrentState(InState)
{
}

TD3DResource::~TD3DResource()
{
}
