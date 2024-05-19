#include "Render.h"
#include "D3DRHI.h"
#include "D3DCommandContent.h"
#include "D3DDescriptorCache.h"
#include "D3DViewport.h"
#include "MeshBatch.h"
#include "MeshComponent.h"
#include "World.h"
#include "PipelineState.h"
#include "Shader.h"
#include "PipelineState.h"
#include "InputLayout.h"
#include "MeshProxy.h"
#include "TextureRepository.h"
#include "MeshRepository.h"
#include "D3DTextureLoader.h"
#include "InputLayout.h"
#include "D3DResourceAllocator.h"
#include "D3DViewAllocator.h"
#include "RenderProxy.h"

bool TRender::Initialize(TD3DRHI* InD3DRHI, TWorld* InWorld)
{
	D3DRHI = InD3DRHI;
	World = InWorld;

	InputLayoutManager = new TInputLayoutManager();
	GraphicsPSOManager = new TGraphicsPSOManager(D3DRHI, InputLayoutManager);
	D3DTextureLoader = new TD3DTextureLoader(D3DRHI);

	this->CreateInputLayouts();

	D3DRHI->GetCommandContent()->ResetCommandList();
	this->CreateMeshProxys();
	this->CreateTextures();
	this->CreateGBuffers();
	D3DRHI->GetCommandContent()->ExecuteCommandList();
	D3DRHI->GetCommandContent()->FlushCommandQueue();

	return true;
}

void TRender::Draw(float dt)
{
	D3DRHI->GetCommandContent()->ResetCommandAllocator();
	D3DRHI->GetCommandContent()->ResetCommandList();

	//SetDescriptorHeaps();
	auto DescriptorCache = D3DRHI->GetCommandContent()->GetDescriptorCache();
	auto CbvSrvUavHeap = DescriptorCache->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	ID3D12DescriptorHeap* DescriptorHeaps[] = { CbvSrvUavHeap.Heap.Get()};
	D3DRHI->GetCommandContent()->GetCommandList()->SetDescriptorHeaps(1, DescriptorHeaps);

	// 更新cbPass
	this->UpdatePassConstants();
	// BasePass
	this->BasePass();
	// 延迟光照
	this->DeferredLightingPass();

	D3DRHI->GetCommandContent()->ExecuteCommandList();
	D3DRHI->GetViewport()->Present();
	D3DRHI->GetCommandContent()->FlushCommandQueue();
}

void TRender::BasePass()
{
	auto ResourceAllocator = D3DRHI->GetDevice()->GetResourceAllocator();

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
		MeshBatch.MeshName = MeshComponent->GetMeshName();
		MeshBatch.InputLayoutName = TMeshRepository::Get().MeshMap.at(MeshComponent->GetMeshName()).InputLayoutName;
		MeshBatch.MeshComponent = MeshComponent;

		TMatrix World = MeshComponent->GetWorldTransform().GetTransformMatrix();
		TMatrix PrevWorld = MeshComponent->GetPrevWorldTransform().GetTransformMatrix();

		TObjectConstants ObjConstant;
		ObjConstant.World = World.Transpose();
		ObjConstant.PrevWorld = PrevWorld.Transpose();

		MeshBatch.ObjConstantsRef = std::make_shared<TD3DResource>();// = ObjConstant;

		TD3DResourceInitInfo ObjConstInitInfo = TD3DResourceInitInfo::Buffer(sizeof(TObjectConstants));
		ObjConstInitInfo.HeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		ObjConstInitInfo.InitState = D3D12_RESOURCE_STATE_COMMON;
		ResourceAllocator->Allocate(ObjConstInitInfo, MeshBatch.ObjConstantsRef.get());

		D3DRHI->UploadBuffer(MeshBatch.ObjConstantsRef.get(), &ObjConstant, sizeof(TObjectConstants));

		//Add to list
		MeshBatchs.emplace_back(MeshBatch);
	}

	// 创建 MeshCommand;
	std::unordered_map<TGraphicsPSODescriptor, std::vector<TMeshCommand>> BaseMeshCommandMap;
	for (const TMeshBatch& MeshBatch : MeshBatchs)
	{
		auto MaterialInstance = MeshBatch.MeshComponent->GetMaterialInstance();
		MaterialInstance->UpdateConstants(D3DRHI);

		TMeshCommand MeshCommand;
		MeshCommand.MeshName = MeshBatch.MeshName;

		// 设置Constant Buffer
		MeshCommand.SetShaderParameter("cbPass", PassConstBufRef.get());
		MeshCommand.SetShaderParameter("cbPerObject", MeshBatch.ObjConstantsRef.get());
		MeshCommand.SetShaderParameter("cbMaterial", MaterialInstance->MaterialConstantsRef.get());

		for (const auto& Pair : MaterialInstance->TextureMap)
		{
			std::string TextureName = Pair.second;

			TD3DShaderResourceView* SRV = D3DTextureMap.at(TextureName)->SRV.get();
			MeshCommand.SetShaderParameter(Pair.first, SRV);
		}

		// Get PSO descriptor of this mesh
		TGraphicsPSODescriptor Descriptor;
		Descriptor.InputLayoutName = MeshBatch.InputLayoutName;
		Descriptor.Shader = MaterialInstance->Parent->GetShaderByDefines(TMacroDefines(), D3DRHI);
		Descriptor.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
		Descriptor.RTVFormats[1] = DXGI_FORMAT_R8G8B8A8_SNORM;
		Descriptor.RTVFormats[2] = DXGI_FORMAT_R32G32B32A32_FLOAT;
		Descriptor.RTVFormats[3] = DXGI_FORMAT_R8G8B8A8_UNORM;
		Descriptor.RTVFormats[4] = DXGI_FORMAT_R16G16_FLOAT;
		Descriptor.RTVFormats[5] = DXGI_FORMAT_R8G8B8A8_UNORM;
		Descriptor.NumRenderTargets = GBufferCount;
		Descriptor.DSVFormat = D3DRHI->GetViewport()->GetViewportInfo().DepthStencilFormat;
		GraphicsPSOManager->TryCreate(Descriptor);

		BaseMeshCommandMap.insert({ Descriptor, std::vector<TMeshCommand>() });
		BaseMeshCommandMap[Descriptor].emplace_back(MeshCommand);
	}

	// Use screen viewport 
	D3D12_VIEWPORT ScreenViewport;
	D3D12_RECT ScissorRect;
	D3DRHI->GetCommandContent()->GetCommandList()->RSSetViewports(1, &ScreenViewport);
	D3DRHI->GetCommandContent()->GetCommandList()->RSSetScissorRects(1, &ScissorRect);

	// 设置BasePass RenderTarget(GBuffer)
	D3DRHI->TransitionResource(GBufferBaseColor->GpuResource.get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	D3DRHI->TransitionResource(GBufferNormal->GpuResource.get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	D3DRHI->TransitionResource(GBufferWorldPos->GpuResource.get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	D3DRHI->TransitionResource(GBufferORM->GpuResource.get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	D3DRHI->TransitionResource(GBufferVelocity->GpuResource.get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	D3DRHI->TransitionResource(GBufferEmissive->GpuResource.get(), D3D12_RESOURCE_STATE_RENDER_TARGET);

	std::vector<TD3DDescriptor> RtvDescriptors;
	RtvDescriptors.push_back(GBufferBaseColor->RTV->GetDescriptor());
	RtvDescriptors.push_back(GBufferNormal->RTV->GetDescriptor());
	RtvDescriptors.push_back(GBufferWorldPos->RTV->GetDescriptor());
	RtvDescriptors.push_back(GBufferORM->RTV->GetDescriptor());
	RtvDescriptors.push_back(GBufferVelocity->RTV->GetDescriptor());
	RtvDescriptors.push_back(GBufferEmissive->RTV->GetDescriptor());

	auto DescriptorCache = D3DRHI->GetCommandContent()->GetDescriptorCache();
	TD3DDescriptor RTVDescriptor;
	DescriptorCache->AppendDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, RtvDescriptors, RTVDescriptor);
	TD3DDescriptor DSVDescriptor;
	DSVDescriptor = D3DRHI->GetViewport()->GetDepthStencilView()->GetDescriptor();

	D3DRHI->GetCommandContent()->GetCommandList()->ClearDepthStencilView(DSVDescriptor.CpuHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	D3DRHI->GetCommandContent()->GetCommandList()->OMSetRenderTargets(GBufferCount, &RTVDescriptor.CpuHandle, true, &DSVDescriptor.CpuHandle);

	// 绘制所有的网格
	for (const auto& Pair : BaseMeshCommandMap)
	{
		const TGraphicsPSODescriptor& PSODescriptor = Pair.first;
		const std::vector<TMeshCommand> MeshCommands = Pair.second;

		// 设置PSO
		D3DRHI->GetCommandContent()->GetCommandList()->SetPipelineState(GraphicsPSOManager->FindAndCreate(PSODescriptor));

		// 设置根签名(应该在绑定参数之前设置)
		TShader* Shader = PSODescriptor.Shader;
		D3DRHI->GetCommandContent()->GetCommandList()->SetGraphicsRootSignature(Shader->RootSignature.Get());

		for (const TMeshCommand& MeshCommand : MeshCommands)
		{
			// 设置着色器参数
			MeshCommand.ApplyShaderParamters(Shader);

			// 绑定着色器参数
			Shader->BindParameters();

			// 获得MeshProxy
			const TMeshProxy& MeshProxy = MeshProxyMap.at(MeshCommand.MeshName);

			// 设置vertex Buffer
			D3D12_VERTEX_BUFFER_VIEW VBView;
			VBView.BufferLocation = MeshProxy.VertexBuffer->D3DResource->GetGPUVirtualAddress();
			VBView.StrideInBytes = MeshProxy.VertexByteStride;
			VBView.SizeInBytes = MeshProxy.VertexBufferByteSize;
			D3DRHI->GetCommandContent()->GetCommandList()->IASetVertexBuffers(0, 1, &VBView);
			// 设置index buffer
			D3D12_INDEX_BUFFER_VIEW IBView;
			IBView.BufferLocation = MeshProxy.VertexBuffer->D3DResource->GetGPUVirtualAddress();
			IBView.Format = MeshProxy.IndexFormat;
			IBView.SizeInBytes = MeshProxy.IndexBufferByteSize;
			D3DRHI->GetCommandContent()->GetCommandList()->IASetIndexBuffer(&IBView);

			// 设置图元类型
			D3DRHI->GetCommandContent()->GetCommandList()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// Draw
			auto& SubMesh = MeshProxy.SubMeshs.at("Default");
			D3DRHI->GetCommandContent()->GetCommandList()->DrawIndexedInstanced(SubMesh.IndexCount, 1, SubMesh.StartIndexLocation, SubMesh.BaseVertexLocation, 0);
		}
	}

	D3DRHI->TransitionResource(GBufferBaseColor->GpuResource.get(), D3D12_RESOURCE_STATE_GENERIC_READ);
	D3DRHI->TransitionResource(GBufferNormal->GpuResource.get(), D3D12_RESOURCE_STATE_GENERIC_READ);
	D3DRHI->TransitionResource(GBufferWorldPos->GpuResource.get(), D3D12_RESOURCE_STATE_GENERIC_READ);
	D3DRHI->TransitionResource(GBufferORM->GpuResource.get(), D3D12_RESOURCE_STATE_GENERIC_READ);
	D3DRHI->TransitionResource(GBufferVelocity->GpuResource.get(), D3D12_RESOURCE_STATE_GENERIC_READ);
	D3DRHI->TransitionResource(GBufferEmissive->GpuResource.get(), D3D12_RESOURCE_STATE_GENERIC_READ);
}

void TRender::DeferredLightingPass()
{
}

void TRender::CreateMeshProxys()
{
	auto ResourceAllocator = D3DRHI->GetDevice()->GetResourceAllocator();

	auto& MeshMap = TMeshRepository::Get().MeshMap;

	for (auto& MeshPair : MeshMap)
	{
		TMesh& Mesh = MeshPair.second;

		// Generate MeshProxy
		MeshProxyMap.emplace(Mesh.MeshName, TMeshProxy());
		TMeshProxy& MeshProxy = MeshProxyMap.at(Mesh.MeshName);

		const UINT VertexBufByteSize = (UINT)Mesh.Vertices.size() * sizeof(TVertex);

		std::vector<std::uint16_t> indices = Mesh.Indices16;
		const UINT IndexBufByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

		// D3DRHI->CreateVertexBuffer(Mesh.Vertices.data(), VertexBufByteSize);
		MeshProxy.VertexBuffer = std::make_shared<TD3DResource>();
		TD3DResourceInitInfo VBufInitInfo = TD3DResourceInitInfo::Buffer(VertexBufByteSize);
		VBufInitInfo.HeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		VBufInitInfo.InitState = D3D12_RESOURCE_STATE_COMMON;
		ResourceAllocator->Allocate(VBufInitInfo, MeshProxy.VertexBuffer.get());
		D3DRHI->UploadBuffer(MeshProxy.VertexBuffer.get(), Mesh.Vertices.data(), VertexBufByteSize);

		// D3DRHI->CreateIndexBuffer(indices.data(), IndexBufByteSize);
		MeshProxy.IndexBuffer = std::make_shared<TD3DResource>();
		TD3DResourceInitInfo IBufInitInfo = TD3DResourceInitInfo::Buffer(IndexBufByteSize);
		IBufInitInfo.HeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		IBufInitInfo.InitState = D3D12_RESOURCE_STATE_COMMON;
		ResourceAllocator->Allocate(IBufInitInfo, MeshProxy.IndexBuffer.get());
		D3DRHI->UploadBuffer(MeshProxy.IndexBuffer.get(), indices.data(), IndexBufByteSize);

		MeshProxy.VertexByteStride = sizeof(TVertex);
		MeshProxy.VertexBufferByteSize = VertexBufByteSize;
		MeshProxy.IndexFormat = DXGI_FORMAT_R16_UINT;
		MeshProxy.IndexBufferByteSize = IndexBufByteSize;

		TSubmeshProxy submesh;
		submesh.IndexCount = (UINT)indices.size();
		submesh.StartIndexLocation = 0;
		submesh.BaseVertexLocation = 0;

		MeshProxy.SubMeshs["Default"] = submesh;
	}
}

#include "FormatConvert.h"
void TRender::CreateTextures()
{
	auto& TextureMap = TTextureRepository::Get().TextureMap;
	for (auto& TexturePair : TextureMap)
	{
		auto& Texture = TexturePair.second;
		D3DTextureMap.emplace(Texture->Name, std::make_shared<TD3DTexture>());

		D3DTextureLoader->Load(D3DTextureMap.at(Texture->Name), Texture->TextureInfo);
	}
}

void TRender::CreateInputLayouts()
{
	//DefaultInputLayout
	std::vector<D3D12_INPUT_ELEMENT_DESC>  DefaultInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	InputLayoutManager->AddInputLayout("DefaultInputLayout", DefaultInputLayout);

	//PositionTexcoordInputLayout
	std::vector<D3D12_INPUT_ELEMENT_DESC> PositionTexcoordInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	InputLayoutManager->AddInputLayout("PositionTexCoordInputLayout", PositionTexcoordInputLayout);
}

void TRender::CreateGBuffers()
{
	auto ResourceAllocator = D3DRHI->GetDevice()->GetResourceAllocator();
	auto ViewAllocator = D3DRHI->GetDevice()->GetViewAllocator();

	int32_t GBufferWidth = D3DRHI->GetViewport()->GetViewportInfo().Width;
	int32_t GBufferHeight = D3DRHI->GetViewport()->GetViewportInfo().Height;

	{
		GBufferBaseColor = std::make_unique<TD3DTexture>();
		TD3DResourceInitInfo BCInitInfo = TD3DResourceInitInfo::Texture2D(GBufferWidth, GBufferHeight, DXGI_FORMAT_R32G32B32A32_FLOAT);
		BCInitInfo.InitState = D3D12_RESOURCE_STATE_COMMON;
		BCInitInfo.HeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		BCInitInfo.ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		ResourceAllocator->Allocate(BCInitInfo, GBufferBaseColor->GpuResource.get());

		D3D12_RENDER_TARGET_VIEW_DESC RTView;
		RTView.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		RTView.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		RTView.Texture2D.MipSlice = 0;
		RTView.Texture2D.PlaneSlice = 0;
		GBufferBaseColor->RTV = std::make_shared<TD3DRenderTargetView>(D3DRHI->GetDevice(), GBufferBaseColor->GpuResource.get());
		ViewAllocator->CreateRenderTargetView(GBufferBaseColor->GpuResource.get(), GBufferBaseColor->RTV.get(), RTView);
	}

	{
		GBufferNormal = std::make_unique<TD3DTexture>();
		TD3DResourceInitInfo BNInitInfo = TD3DResourceInitInfo::Texture2D(GBufferWidth, GBufferHeight, DXGI_FORMAT_R8G8B8A8_SNORM);
		BNInitInfo.InitState = D3D12_RESOURCE_STATE_COMMON;
		BNInitInfo.HeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		BNInitInfo.ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		ResourceAllocator->Allocate(BNInitInfo, GBufferNormal->GpuResource.get());

		D3D12_RENDER_TARGET_VIEW_DESC RTView;
		RTView.Format = DXGI_FORMAT_R8G8B8A8_SNORM;
		RTView.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		RTView.Texture2D.MipSlice = 0;
		RTView.Texture2D.PlaneSlice = 0;
		GBufferNormal->RTV = std::make_shared<TD3DRenderTargetView>(D3DRHI->GetDevice(), GBufferNormal->GpuResource.get());
		ViewAllocator->CreateRenderTargetView(GBufferNormal->GpuResource.get(), GBufferNormal->RTV.get(), RTView);
	}

	{
		GBufferWorldPos = std::make_unique<TD3DTexture>();
		TD3DResourceInitInfo BWInitInfo = TD3DResourceInitInfo::Texture2D(GBufferWidth, GBufferHeight, DXGI_FORMAT_R32G32B32A32_FLOAT);
		BWInitInfo.InitState = D3D12_RESOURCE_STATE_COMMON;
		BWInitInfo.HeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		BWInitInfo.ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		ResourceAllocator->Allocate(BWInitInfo, GBufferWorldPos->GpuResource.get());

		D3D12_RENDER_TARGET_VIEW_DESC RTView;
		RTView.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		RTView.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		RTView.Texture2D.MipSlice = 0;
		RTView.Texture2D.PlaneSlice = 0;
		GBufferWorldPos->RTV = std::make_shared<TD3DRenderTargetView>(D3DRHI->GetDevice(), GBufferWorldPos->GpuResource.get());
		ViewAllocator->CreateRenderTargetView(GBufferWorldPos->GpuResource.get(), GBufferWorldPos->RTV.get(), RTView);
	}


	{
		GBufferORM = std::make_unique<TD3DTexture>();
		TD3DResourceInitInfo BROMInitInfo = TD3DResourceInitInfo::Texture2D(GBufferWidth, GBufferHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
		BROMInitInfo.InitState = D3D12_RESOURCE_STATE_COMMON;
		BROMInitInfo.HeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		BROMInitInfo.ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		ResourceAllocator->Allocate(BROMInitInfo, GBufferORM->GpuResource.get());

		D3D12_RENDER_TARGET_VIEW_DESC RTView;
		RTView.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		RTView.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		RTView.Texture2D.MipSlice = 0;
		RTView.Texture2D.PlaneSlice = 0;
		GBufferORM->RTV = std::make_shared<TD3DRenderTargetView>(D3DRHI->GetDevice(), GBufferORM->GpuResource.get());
		ViewAllocator->CreateRenderTargetView(GBufferORM->GpuResource.get(), GBufferORM->RTV.get(), RTView);
	}

	{
		GBufferVelocity = std::make_unique<TD3DTexture>();
		TD3DResourceInitInfo BVInitInfo = TD3DResourceInitInfo::Texture2D(GBufferWidth, GBufferHeight, DXGI_FORMAT_R16G16_FLOAT);
		BVInitInfo.InitState = D3D12_RESOURCE_STATE_COMMON;
		BVInitInfo.HeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		BVInitInfo.ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		ResourceAllocator->Allocate(BVInitInfo, GBufferVelocity->GpuResource.get());

		D3D12_RENDER_TARGET_VIEW_DESC RTView;
		RTView.Format = DXGI_FORMAT_R16G16_FLOAT;
		RTView.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		RTView.Texture2D.MipSlice = 0;
		RTView.Texture2D.PlaneSlice = 0;
		GBufferVelocity->RTV = std::make_shared<TD3DRenderTargetView>(D3DRHI->GetDevice(), GBufferVelocity->GpuResource.get());
		ViewAllocator->CreateRenderTargetView(GBufferVelocity->GpuResource.get(), GBufferVelocity->RTV.get(), RTView);
	}

	{
		GBufferEmissive = std::make_unique<TD3DTexture>();
		TD3DResourceInitInfo BEInitInfo = TD3DResourceInitInfo::Texture2D(GBufferWidth, GBufferHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
		BEInitInfo.InitState = D3D12_RESOURCE_STATE_COMMON;
		BEInitInfo.HeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		BEInitInfo.ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		ResourceAllocator->Allocate(BEInitInfo, GBufferEmissive->GpuResource.get());

		D3D12_RENDER_TARGET_VIEW_DESC RTView;
		RTView.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		RTView.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		RTView.Texture2D.MipSlice = 0;
		RTView.Texture2D.PlaneSlice = 0;
		GBufferEmissive->RTV = std::make_shared<TD3DRenderTargetView>(D3DRHI->GetDevice(), GBufferEmissive->GpuResource.get());
		ViewAllocator->CreateRenderTargetView(GBufferEmissive->GpuResource.get(), GBufferEmissive->RTV.get(), RTView);
	}
}

void TRender::UpdatePassConstants()
{
	PassConstBufRef = std::make_shared<TD3DResource>();
	
	// TODO Camera
	TMatrix View = TMatrix::Identity;
	TMatrix Proj = TMatrix::Identity;

	TPassConstants PassConstants;
	PassConstants.View = View;
	PassConstants.Proj = Proj;

	auto ResourceAllocator = D3DRHI->GetDevice()->GetResourceAllocator();

	TD3DResourceInitInfo PassCBInitInfo = TD3DResourceInitInfo::Buffer(sizeof(TPassConstants));
	PassCBInitInfo.HeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	PassCBInitInfo.InitState = D3D12_RESOURCE_STATE_COMMON;
	ResourceAllocator->Allocate(PassCBInitInfo, PassConstBufRef.get());

	D3DRHI->UploadBuffer(PassConstBufRef.get(), &PassConstants, sizeof(TPassConstants));
}
