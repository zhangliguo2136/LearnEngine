#include "Render.h"
#include "D3DRHI.h"
#include "D3DCommandContent.h"
#include "D3DDescriptorCache.h"
#include "D3DViewport.h"
#include "MeshBatch.h"
#include "MeshComponent.h"
#include "World.h"

void TRender::Draw(float dt)
{
	D3DRHI->GetCommandContent()->ResetCommandAllocator();
	D3DRHI->GetCommandContent()->ResetCommandList();

	//SetDescriptorHeaps();
	auto DescriptorCache = D3DRHI->GetCommandContent()->GetDescriptorCache();
	auto CbvSrvUavHeap = DescriptorCache->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	ID3D12DescriptorHeap* DescriptorHeaps[] = { CbvSrvUavHeap.Heap.Get()};
	D3DRHI->GetCommandContent()->GetCommandList()->SetDescriptorHeaps(1, DescriptorHeaps);

	// BasePass
	this->BasePass();

	this->DeferredLightingPass();

	D3DRHI->GetCommandContent()->ExecuteCommands();
	D3DRHI->GetViewport()->Present();
	D3DRHI->GetCommandContent()->FlushCommandQueue();
}

void TRender::BasePass()
{
	// GatherAllMeshBatchs
	std::vector<TMeshBatch> MeshBatchs;
	auto Actors = World->GetActors();
	std::vector<TMeshComponent*> AllMeshComponents;
	for (auto Actor : Actors)
	{
		auto MeshComponents = Actor->GetComponentsOfClass<TMeshComponent>();
		for (auto MeshComponent : MeshComponents)
		{
			AllMeshComponents.push_back(MeshComponent);
		}
	}
	// Generate MeshBatchs
	for (auto MeshComponent : AllMeshComponents)
	{
		TMeshBatch MeshBatch;
		MeshBatch.MeshComponent = MeshComponent;

		//Add to list
		MeshBatchs.emplace_back(MeshBatch);
	}
}

void TRender::DeferredLightingPass()
{
}
