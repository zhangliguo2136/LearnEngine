#include "Render.h"
#include "D3DRHI.h"
#include "D3DCommandContent.h"
#include "D3DDescriptorCache.h"
#include "D3DView.h"
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
#include "DirectionalLightActor.h"
#include "CameraComponent.h"
#include "CameraActor.h"

bool TRender::Initialize(TD3DRHI* InD3DRHI, TWorld* InWorld)
{
	D3DRHI = InD3DRHI;
	World = InWorld;

	InputLayoutManager = new TInputLayoutManager();
	GraphicsPSOManager = new TGraphicsPSOManager(D3DRHI, InputLayoutManager);
	D3DTextureLoader = new TD3DTextureLoader(D3DRHI);

	this->CreateInputLayouts();
	this->CreateGlobalPipelineState();

	D3DRHI->GetCommandContent()->ResetCommandList();
	this->CreateMeshProxys();
	this->CreateTextures();
	this->CreateGBuffers();
	this->CreateColorTextures();
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
	// 更新光照参数
	this->UpdateLightParameters();
	// BasePass
	this->BasePass();
	// 延迟光照
	this->DeferredLightingPass();
	// 后处理
	this->PostProcessPass();

	D3DRHI->GetCommandContent()->ExecuteCommandList();
	D3DRHI->GetViewport()->Present();
	D3DRHI->GetCommandContent()->FlushCommandQueue();

	DescriptorCache->Clear();
}

void TRender::BasePass()
{
	auto ResourceAllocator = D3DRHI->GetDevice()->GetResourceAllocator();

	// GatherAllMeshBatchs
	MeshBatchs.clear();
	BaseMeshCommandMap.clear();

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

		TD3DResourceInitInfo ObjConstInitInfo = TD3DResourceInitInfo::Buffer_Upload(sizeof(TObjectConstants));
		ResourceAllocator->Allocate(ObjConstInitInfo, MeshBatch.ObjConstantsRef.get());

		void* MappedData = nullptr;
		MeshBatch.ObjConstantsRef->D3DResource->Map(0, nullptr, &MappedData);
		memcpy_s(MappedData, sizeof(TObjectConstants), &ObjConstant, sizeof(TObjectConstants));
		MeshBatch.ObjConstantsRef->D3DResource->Unmap(0, nullptr);

		//Add to list
		MeshBatchs.emplace_back(MeshBatch);
	}

	// 创建 MeshCommand;
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

	auto CommandList = D3DRHI->GetCommandContent()->GetCommandList();

	// Use screen viewport 
	D3D12_VIEWPORT ScreenViewport;
	D3D12_RECT ScissorRect;
	D3DRHI->GetViewport()->GetD3DViewport(ScreenViewport, ScissorRect);
	CommandList->RSSetViewports(1, &ScreenViewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);

	// 设置BasePass RenderTarget(GBuffer)
	D3DRHI->TransitionResource(GBufferBaseColor->GpuResource.get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	D3DRHI->TransitionResource(GBufferNormal->GpuResource.get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	D3DRHI->TransitionResource(GBufferWorldPos->GpuResource.get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	D3DRHI->TransitionResource(GBufferORM->GpuResource.get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	D3DRHI->TransitionResource(GBufferVelocity->GpuResource.get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	D3DRHI->TransitionResource(GBufferEmissive->GpuResource.get(), D3D12_RESOURCE_STATE_RENDER_TARGET);

	// Clear renderTargets
	const float ClearValue[4] = { 0.f, 0.f, 0.f, 1.f };
	CommandList->ClearRenderTargetView(GBufferBaseColor->RTV->GetCpuDescriptorHandle(), ClearValue, 0, nullptr);
	CommandList->ClearRenderTargetView(GBufferNormal->RTV->GetCpuDescriptorHandle(), ClearValue, 0, nullptr);
	CommandList->ClearRenderTargetView(GBufferWorldPos->RTV->GetCpuDescriptorHandle(), ClearValue, 0, nullptr);
	CommandList->ClearRenderTargetView(GBufferORM->RTV->GetCpuDescriptorHandle(), ClearValue, 0, nullptr);
	CommandList->ClearRenderTargetView(GBufferVelocity->RTV->GetCpuDescriptorHandle(), ClearValue, 0, nullptr);
	CommandList->ClearRenderTargetView(GBufferEmissive->RTV->GetCpuDescriptorHandle(), ClearValue, 0, nullptr);


	std::vector<TD3DDescriptor> RTVDescriptors;
	RTVDescriptors.push_back(GBufferBaseColor->RTV->GetDescriptor());
	RTVDescriptors.push_back(GBufferNormal->RTV->GetDescriptor());
	RTVDescriptors.push_back(GBufferWorldPos->RTV->GetDescriptor());
	RTVDescriptors.push_back(GBufferORM->RTV->GetDescriptor());
	RTVDescriptors.push_back(GBufferVelocity->RTV->GetDescriptor());
	RTVDescriptors.push_back(GBufferEmissive->RTV->GetDescriptor());

	auto DescriptorCache = D3DRHI->GetCommandContent()->GetDescriptorCache();

	TD3DDescriptor RTVDescriptor;
	DescriptorCache->AppendDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, RTVDescriptors, RTVDescriptor);

	TD3DDescriptor DSVDescriptor;
	DSVDescriptor = D3DRHI->GetViewport()->GetDepthStencilView()->GetDescriptor();

	CommandList->ClearDepthStencilView(DSVDescriptor.CpuHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	CommandList->OMSetRenderTargets(GBufferCount, &RTVDescriptor.CpuHandle, true, &DSVDescriptor.CpuHandle);

	// 绘制所有的网格
	for (const auto& Pair : BaseMeshCommandMap)
	{
		const TGraphicsPSODescriptor& PSODescriptor = Pair.first;
		const std::vector<TMeshCommand> MeshCommands = Pair.second;

		// 设置PSO
		CommandList->SetPipelineState(GraphicsPSOManager->FindAndCreate(PSODescriptor));

		// 设置根签名(应该在绑定参数之前设置)
		TShader* Shader = PSODescriptor.Shader;
		CommandList->SetGraphicsRootSignature(Shader->RootSignature.Get());

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
			IBView.BufferLocation = MeshProxy.IndexBuffer->D3DResource->GetGPUVirtualAddress();
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
	D3DRHI->TransitionResource(ColorTexture->GpuResource.get(), D3D12_RESOURCE_STATE_RENDER_TARGET);

	D3D12_VIEWPORT ScreenViewport;
	D3D12_RECT ScissorRect;
	D3DRHI->GetViewport()->GetD3DViewport(ScreenViewport, ScissorRect);
	D3DRHI->GetCommandContent()->GetCommandList()->RSSetViewports(1, &ScreenViewport);
	D3DRHI->GetCommandContent()->GetCommandList()->RSSetScissorRects(1, &ScissorRect);

	// Clear the ColorTexture and depth buffer.
	auto RTVHandle = ColorTexture->RTV->GetCpuDescriptorHandle();
	TD3DDescriptor DSVDescriptor;
	DSVDescriptor = D3DRHI->GetViewport()->GetDepthStencilView()->GetDescriptor();

	const float ClearValue[4] = { 0.f, 0.f, 0.f, 1.f };
	D3DRHI->GetCommandContent()->GetCommandList()->ClearRenderTargetView(RTVHandle, ClearValue, 0, nullptr);
	// Specify the buffers we are going to render to.
	D3DRHI->GetCommandContent()->GetCommandList()->OMSetRenderTargets(1, &RTVHandle, true, &DSVDescriptor.CpuHandle);

	// Set DeferredLighting PSO
	D3DRHI->GetCommandContent()->GetCommandList()->SetPipelineState(GraphicsPSOManager->FindAndCreate(DeferredLightingPSODescriptor));

	// Set RootSignature
	TShader* Shader = DeferredLightingPSODescriptor.Shader;
	D3DRHI->GetCommandContent()->GetCommandList()->SetGraphicsRootSignature(Shader->RootSignature.Get()); //should before binding
	
	//-------------------------------------Set paramters-------------------------------------------
	Shader->SetParameter("cbPass", PassConstBufRef.get());

	Shader->SetParameter("BaseColorGbuffer", GBufferBaseColor->SRV.get());
	Shader->SetParameter("NormalGbuffer", GBufferNormal->SRV.get());
	Shader->SetParameter("WorldPosGbuffer", GBufferWorldPos->SRV.get());
	Shader->SetParameter("OrmGbuffer", GBufferORM->SRV.get());
	Shader->SetParameter("EmissiveGbuffer", GBufferEmissive->SRV.get());

	Shader->SetParameter("LightCommon", LightCommonBufRef.get());
	Shader->SetParameter("Lights", LightParametersBufRef->SRV.get());

	// Bind paramters
	Shader->BindParameters();

	// Draw ScreenQuad
	{
		const TMeshProxy& MeshProxy = MeshProxyMap.at("ScreenQuadMesh");

		// 设置vertex Buffer
		D3D12_VERTEX_BUFFER_VIEW VBView;
		VBView.BufferLocation = MeshProxy.VertexBuffer->D3DResource->GetGPUVirtualAddress();
		VBView.StrideInBytes = MeshProxy.VertexByteStride;
		VBView.SizeInBytes = MeshProxy.VertexBufferByteSize;
		D3DRHI->GetCommandContent()->GetCommandList()->IASetVertexBuffers(0, 1, &VBView);
		// 设置index buffer
		D3D12_INDEX_BUFFER_VIEW IBView;
		IBView.BufferLocation = MeshProxy.IndexBuffer->D3DResource->GetGPUVirtualAddress();
		IBView.Format = MeshProxy.IndexFormat;
		IBView.SizeInBytes = MeshProxy.IndexBufferByteSize;
		D3DRHI->GetCommandContent()->GetCommandList()->IASetIndexBuffer(&IBView);


		D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		D3DRHI->GetCommandContent()->GetCommandList()->IASetPrimitiveTopology(PrimitiveType);

		// Draw 
		auto& SubMesh = MeshProxy.SubMeshs.at("Default");
		D3DRHI->GetCommandContent()->GetCommandList()->DrawIndexedInstanced(SubMesh.IndexCount, 1, SubMesh.StartIndexLocation, SubMesh.BaseVertexLocation, 0);
	}

	// Transition to PRESENT state.
	D3DRHI->TransitionResource(ColorTexture->GpuResource.get(), D3D12_RESOURCE_STATE_PRESENT);
}

void TRender::PostProcessPass()
{
	auto CommandList = D3DRHI->GetCommandContent()->GetCommandList();
	TD3DTextureRef CurrRenderTarget = D3DRHI->GetViewport()->GetCurrentBackRT();

	D3DRHI->TransitionResource(ColorTexture->GpuResource.get(), D3D12_RESOURCE_STATE_GENERIC_READ);

	D3DRHI->TransitionResource(CurrRenderTarget->GpuResource.get(), D3D12_RESOURCE_STATE_RENDER_TARGET);

	D3D12_VIEWPORT ScreenViewport;
	D3D12_RECT ScissorRect;
	D3DRHI->GetViewport()->GetD3DViewport(ScreenViewport, ScissorRect);
	CommandList->RSSetViewports(1, &ScreenViewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);

	// Clear the back buffer.
	float ClearColor[4] = {0.f, 0.f, 0.f, 1.0f};
	CommandList->ClearRenderTargetView(CurrRenderTarget->RTV->GetCpuDescriptorHandle(), ClearColor, 0, nullptr);

	// Specify the buffers we are going to render to.
	TD3DDescriptor RTDescriptor = CurrRenderTarget->RTV->GetDescriptor();
	CommandList->OMSetRenderTargets(1, &RTDescriptor.CpuHandle, true, nullptr);

	// Set PSO
	CommandList->SetPipelineState(GraphicsPSOManager->FindAndCreate(PostProcessPSODescriptor));

	// Set RootSignature
	CommandList->SetGraphicsRootSignature(PostProcessShader->RootSignature.Get()); //should before binding

	// Set paramters
	//PostProcessShader->SetParameter("cbPass", BasePassCBRef);
	PostProcessShader->SetParameter("ColorTexture", ColorTexture->SRV.get());

	// Bind paramters
	PostProcessShader->BindParameters();

	// Draw ScreenQuad
	{
		const TMeshProxy& MeshProxy = MeshProxyMap.at("ScreenQuadMesh");

		// 设置vertex Buffer
		D3D12_VERTEX_BUFFER_VIEW VBView;
		VBView.BufferLocation = MeshProxy.VertexBuffer->D3DResource->GetGPUVirtualAddress();
		VBView.StrideInBytes = MeshProxy.VertexByteStride;
		VBView.SizeInBytes = MeshProxy.VertexBufferByteSize;
		CommandList->IASetVertexBuffers(0, 1, &VBView);
		// 设置index buffer
		D3D12_INDEX_BUFFER_VIEW IBView;
		IBView.BufferLocation = MeshProxy.IndexBuffer->D3DResource->GetGPUVirtualAddress();
		IBView.Format = MeshProxy.IndexFormat;
		IBView.SizeInBytes = MeshProxy.IndexBufferByteSize;
		CommandList->IASetIndexBuffer(&IBView);

		D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		CommandList->IASetPrimitiveTopology(PrimitiveType);

		// Draw 
		auto& SubMesh = MeshProxy.SubMeshs.at("Default");
		CommandList->DrawIndexedInstanced(SubMesh.IndexCount, 1, SubMesh.StartIndexLocation, SubMesh.BaseVertexLocation, 0);
	}

	// Transition back-buffer to PRESENT state.
	D3DRHI->TransitionResource(CurrRenderTarget->GpuResource.get(), D3D12_RESOURCE_STATE_PRESENT);
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

		MeshProxy.VertexBuffer = std::make_shared<TD3DResource>();
		TD3DResourceInitInfo VBufInitInfo = TD3DResourceInitInfo::Buffer_Default(VertexBufByteSize);
		ResourceAllocator->Allocate(VBufInitInfo, MeshProxy.VertexBuffer.get());
		D3DRHI->UploadBuffer(MeshProxy.VertexBuffer.get(), Mesh.Vertices.data(), VertexBufByteSize);

		// D3DRHI->CreateIndexBuffer(indices.data(), IndexBufByteSize);
		MeshProxy.IndexBuffer = std::make_shared<TD3DResource>();
		TD3DResourceInitInfo IBufInitInfo = TD3DResourceInitInfo::Buffer_Default(IndexBufByteSize);
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
		GBufferBaseColor->RTV = std::make_shared<TD3DRenderTargetView>(D3DRHI->GetDevice(), GBufferBaseColor->GpuResource.get());
		GBufferBaseColor->SRV = std::make_shared<TD3DShaderResourceView>(D3DRHI->GetDevice(), GBufferBaseColor->GpuResource.get());

		TD3DResourceInitInfo BCInitInfo = TD3DResourceInitInfo::Texture2D(GBufferWidth, GBufferHeight, DXGI_FORMAT_R32G32B32A32_FLOAT);
		BCInitInfo.ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		ResourceAllocator->Allocate(BCInitInfo, GBufferBaseColor->GpuResource.get());

		TD3DViewInitInfo RTView = TD3DViewInitInfo::RTView_Texture2D(GBufferBaseColor->GpuResource.get(), DXGI_FORMAT_R32G32B32A32_FLOAT);
		ViewAllocator->Allocate(GBufferBaseColor->RTV.get(), RTView);

		TD3DViewInitInfo SRView = TD3DViewInitInfo::SRView_Texture2D(GBufferBaseColor->GpuResource.get(), DXGI_FORMAT_R32G32B32A32_FLOAT);
		ViewAllocator->Allocate(GBufferBaseColor->SRV.get(), SRView);
	}

	{
		GBufferNormal = std::make_unique<TD3DTexture>();
		GBufferNormal->RTV = std::make_shared<TD3DRenderTargetView>(D3DRHI->GetDevice(), GBufferNormal->GpuResource.get());
		GBufferNormal->SRV = std::make_shared<TD3DShaderResourceView>(D3DRHI->GetDevice(), GBufferNormal->GpuResource.get());

		TD3DResourceInitInfo BNInitInfo = TD3DResourceInitInfo::Texture2D(GBufferWidth, GBufferHeight, DXGI_FORMAT_R8G8B8A8_SNORM);
		BNInitInfo.ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		ResourceAllocator->Allocate(BNInitInfo, GBufferNormal->GpuResource.get());

		TD3DViewInitInfo RTView = TD3DViewInitInfo::RTView_Texture2D(GBufferNormal->GpuResource.get(), DXGI_FORMAT_R8G8B8A8_SNORM);
		ViewAllocator->Allocate(GBufferNormal->RTV.get(), RTView);

		TD3DViewInitInfo SRView = TD3DViewInitInfo::SRView_Texture2D(GBufferNormal->GpuResource.get(), DXGI_FORMAT_R8G8B8A8_SNORM);
		ViewAllocator->Allocate(GBufferNormal->SRV.get(), SRView);
	}

	{
		GBufferWorldPos = std::make_unique<TD3DTexture>();
		GBufferWorldPos->RTV = std::make_shared<TD3DRenderTargetView>(D3DRHI->GetDevice(), GBufferWorldPos->GpuResource.get());
		GBufferWorldPos->SRV = std::make_shared<TD3DShaderResourceView>(D3DRHI->GetDevice(), GBufferWorldPos->GpuResource.get());

		TD3DResourceInitInfo BWInitInfo = TD3DResourceInitInfo::Texture2D(GBufferWidth, GBufferHeight, DXGI_FORMAT_R32G32B32A32_FLOAT);
		BWInitInfo.ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		ResourceAllocator->Allocate(BWInitInfo, GBufferWorldPos->GpuResource.get());

		TD3DViewInitInfo RTView = TD3DViewInitInfo::RTView_Texture2D(GBufferWorldPos->GpuResource.get(), DXGI_FORMAT_R32G32B32A32_FLOAT);
		ViewAllocator->Allocate(GBufferWorldPos->RTV.get(), RTView);

		TD3DViewInitInfo SRView = TD3DViewInitInfo::SRView_Texture2D(GBufferWorldPos->GpuResource.get(), DXGI_FORMAT_R32G32B32A32_FLOAT);
		ViewAllocator->Allocate(GBufferWorldPos->SRV.get(), SRView);

	}

	{
		GBufferORM = std::make_unique<TD3DTexture>();
		GBufferORM->RTV = std::make_shared<TD3DRenderTargetView>(D3DRHI->GetDevice(), GBufferORM->GpuResource.get());
		GBufferORM->SRV = std::make_shared<TD3DShaderResourceView>(D3DRHI->GetDevice(), GBufferORM->GpuResource.get());

		TD3DResourceInitInfo BROMInitInfo = TD3DResourceInitInfo::Texture2D(GBufferWidth, GBufferHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
		BROMInitInfo.ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		ResourceAllocator->Allocate(BROMInitInfo, GBufferORM->GpuResource.get());

		TD3DViewInitInfo RTView = TD3DViewInitInfo::RTView_Texture2D(GBufferORM->GpuResource.get(), DXGI_FORMAT_R8G8B8A8_UNORM);
		ViewAllocator->Allocate(GBufferORM->RTV.get(), RTView);

		TD3DViewInitInfo SRView = TD3DViewInitInfo::SRView_Texture2D(GBufferORM->GpuResource.get(), DXGI_FORMAT_R8G8B8A8_UNORM);
		ViewAllocator->Allocate(GBufferORM->SRV.get(), SRView);
	}

	{
		GBufferVelocity = std::make_unique<TD3DTexture>();
		GBufferVelocity->RTV = std::make_shared<TD3DRenderTargetView>(D3DRHI->GetDevice(), GBufferVelocity->GpuResource.get());
		GBufferVelocity->SRV = std::make_shared<TD3DShaderResourceView>(D3DRHI->GetDevice(), GBufferVelocity->GpuResource.get());

		TD3DResourceInitInfo BVInitInfo = TD3DResourceInitInfo::Texture2D(GBufferWidth, GBufferHeight, DXGI_FORMAT_R16G16_FLOAT);
		BVInitInfo.ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		ResourceAllocator->Allocate(BVInitInfo, GBufferVelocity->GpuResource.get());

		TD3DViewInitInfo RTView = TD3DViewInitInfo::RTView_Texture2D(GBufferVelocity->GpuResource.get(), DXGI_FORMAT_R16G16_FLOAT);
		ViewAllocator->Allocate(GBufferVelocity->RTV.get(), RTView);

		TD3DViewInitInfo SRView = TD3DViewInitInfo::SRView_Texture2D(GBufferVelocity->GpuResource.get(), DXGI_FORMAT_R16G16_FLOAT);
		ViewAllocator->Allocate(GBufferVelocity->SRV.get(), SRView);

	}

	{
		GBufferEmissive = std::make_unique<TD3DTexture>();
		GBufferEmissive->RTV = std::make_shared<TD3DRenderTargetView>(D3DRHI->GetDevice(), GBufferEmissive->GpuResource.get());
		GBufferEmissive->SRV = std::make_shared<TD3DShaderResourceView>(D3DRHI->GetDevice(), GBufferEmissive->GpuResource.get());

		TD3DResourceInitInfo BEInitInfo = TD3DResourceInitInfo::Texture2D(GBufferWidth, GBufferHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
		BEInitInfo.ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		ResourceAllocator->Allocate(BEInitInfo, GBufferEmissive->GpuResource.get());

		TD3DViewInitInfo RTView = TD3DViewInitInfo::RTView_Texture2D(GBufferEmissive->GpuResource.get(), DXGI_FORMAT_R8G8B8A8_UNORM);
		ViewAllocator->Allocate(GBufferEmissive->RTV.get(), RTView);

		TD3DViewInitInfo SRView = TD3DViewInitInfo::SRView_Texture2D(GBufferEmissive->GpuResource.get(), DXGI_FORMAT_R8G8B8A8_UNORM);
		ViewAllocator->Allocate(GBufferEmissive->SRV.get(), SRView);
	}
}

void TRender::CreateGlobalPipelineState()
{
	TShaderInfo DeferInfo;
	DeferInfo.ShaderName = "DeferredLighting";
	DeferInfo.FileName = "DeferredLighting";
	DeferInfo.bCreateVS = true;
	DeferInfo.bCreatePS = true;
	DeferredLightingShader = std::make_unique<TShader>(DeferInfo, D3DRHI);

	{
		D3D12_DEPTH_STENCIL_DESC DSD = TGraphicsPSODescriptor::DefaultDepthStencil();
		{
			DSD.DepthEnable = FALSE;
			DSD.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
			DSD.StencilEnable = TRUE;
			DSD.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
			DSD.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		}

		D3D12_BLEND_DESC BlendState = TGraphicsPSODescriptor::DefaultBlend();
		{
			BlendState.RenderTarget[0].BlendEnable = TRUE;
			BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		}

		D3D12_RASTERIZER_DESC RasterizerDesc = TGraphicsPSODescriptor::DefaultRasterizer();
		{
			RasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
			RasterizerDesc.DepthClipEnable = FALSE;
		}

		DeferredLightingPSODescriptor.InputLayoutName = std::string("DefaultInputLayout");
		DeferredLightingPSODescriptor.Shader = DeferredLightingShader.get();
		DeferredLightingPSODescriptor.BlendDesc = BlendState;
		DeferredLightingPSODescriptor.DepthStencilDesc = DSD;
		DeferredLightingPSODescriptor.RasterizerDesc = RasterizerDesc;
		DeferredLightingPSODescriptor.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
		DeferredLightingPSODescriptor.NumRenderTargets = 1;

		GraphicsPSOManager->TryCreate(DeferredLightingPSODescriptor);
	}


	TShaderInfo PostProcessInfo;
	PostProcessInfo.ShaderName = "PostProcess";
	PostProcessInfo.FileName = "PostProcess";
	PostProcessInfo.bCreateVS = true;
	PostProcessInfo.bCreatePS = true;
	PostProcessShader = std::make_unique<TShader>(PostProcessInfo, D3DRHI);

	{
		D3D12_DEPTH_STENCIL_DESC DSD = TGraphicsPSODescriptor::DefaultDepthStencil();
		{
			DSD.DepthEnable = FALSE;
		}

		D3D12_RASTERIZER_DESC RasterizerDesc = TGraphicsPSODescriptor::DefaultRasterizer();
		{
			RasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
		}

		PostProcessPSODescriptor.InputLayoutName = std::string("DefaultInputLayout");
		PostProcessPSODescriptor.Shader = PostProcessShader.get();
		PostProcessPSODescriptor.DepthStencilDesc = DSD;
		PostProcessPSODescriptor.RasterizerDesc = RasterizerDesc;
		PostProcessPSODescriptor.NumRenderTargets = 1;
		PostProcessPSODescriptor.RTVFormats[0] = D3DRHI->GetViewport()->GetViewportInfo().BackBufferFormat;
		PostProcessPSODescriptor.DSVFormat = DXGI_FORMAT_UNKNOWN;

		GraphicsPSOManager->TryCreate(PostProcessPSODescriptor);
	}
}

void TRender::CreateColorTextures()
{
	auto ResourceAllocator = D3DRHI->GetDevice()->GetResourceAllocator();
	auto ViewAllocator = D3DRHI->GetDevice()->GetViewAllocator();

	int32_t ColorTextureWidth = D3DRHI->GetViewport()->GetViewportInfo().Width;
	int32_t ColorTextureHeight = D3DRHI->GetViewport()->GetViewportInfo().Height;

	{
		ColorTexture = std::make_unique<TD3DTexture>();
		ColorTexture->RTV = std::make_shared<TD3DRenderTargetView>(D3DRHI->GetDevice(), ColorTexture->GpuResource.get());
		ColorTexture->SRV = std::make_shared<TD3DShaderResourceView>(D3DRHI->GetDevice(), ColorTexture->GpuResource.get());

		TD3DResourceInitInfo BCInitInfo = TD3DResourceInitInfo::Texture2D(ColorTextureWidth, ColorTextureHeight, DXGI_FORMAT_R32G32B32A32_FLOAT);
		BCInitInfo.ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		ResourceAllocator->Allocate(BCInitInfo, ColorTexture->GpuResource.get());

		TD3DViewInitInfo RTVInfo = TD3DViewInitInfo::RTView_Texture2D(ColorTexture->GpuResource.get(), DXGI_FORMAT_R32G32B32A32_FLOAT);
		ViewAllocator->Allocate(ColorTexture->RTV.get(), RTVInfo);

		TD3DViewInitInfo SRVInfo = TD3DViewInitInfo::SRView_Texture2D(ColorTexture->GpuResource.get(), DXGI_FORMAT_R32G32B32A32_FLOAT);
		ViewAllocator->Allocate(ColorTexture->SRV.get(), SRVInfo);
	}
}

void TRender::UpdatePassConstants()
{
	PassConstBufRef = std::make_shared<TD3DResource>();
	
	// TODO Camera
	TCameraComponent* CameraComponent = nullptr;
	auto CameraActors = World->GetAllActorsOfClass<TCameraActor>();
	for (auto CameraActor : CameraActors)
	{
		CameraComponent = CameraActor->GetCameraComponent();
		break;
	}
	TMatrix View = CameraComponent->GetView();
	TMatrix Proj = CameraComponent->GetProj();
	TMatrix ViewProj = View * Proj;
	TMatrix PrevViewProj = CameraComponent->GetPrevViewProj();
	TVector3f EyePosW = CameraComponent->GetWorldLocation();

	TPassConstants PassConstants;
	PassConstants.View = View.Transpose();
	PassConstants.Proj = Proj.Transpose();
	PassConstants.ViewProj = ViewProj.Transpose();
	PassConstants.PrevViewProj = PrevViewProj.Transpose();
	PassConstants.EyePosW = EyePosW;

	auto ResourceAllocator = D3DRHI->GetDevice()->GetResourceAllocator();

	TD3DResourceInitInfo PassCBInitInfo = TD3DResourceInitInfo::Buffer_Upload(sizeof(TPassConstants));
	ResourceAllocator->Allocate(PassCBInitInfo, PassConstBufRef.get());

	void* MappedData = nullptr;
	PassConstBufRef->D3DResource->Map(0, nullptr, &MappedData);
	memcpy_s(MappedData, sizeof(TPassConstants), &PassConstants, sizeof(TPassConstants));
	PassConstBufRef->D3DResource->Unmap(0, nullptr);
}

void TRender::UpdateLightParameters()
{
	std::vector<TLightParameters> LightParametersArray;

	auto Lights = World->GetAllActorsOfClass<TLightActor>();
	for (auto LightIdx = 0; LightIdx < Lights.size(); LightIdx++)
	{
		auto Light = Lights[LightIdx];

		if (Light->GetType() == ELightType::DirectionalLight)
		{
			auto DirectionalLight = dynamic_cast<TDirectionalLightActor*>(Light);

			TLightParameters LightParameter;
			LightParameter.Color = DirectionalLight->GetLightColor();
			LightParameter.Intensity = DirectionalLight->GetLightIntensity();
			LightParameter.Direction = DirectionalLight->GetLightDirection();
			LightParameter.LightType = ELightType::DirectionalLight;

			LightParametersArray.push_back(LightParameter);
		}
	}

	uint32_t ElementCount = LightParametersArray.size();
	uint32_t ElementSize = (uint32_t)(sizeof(TLightParameters));

	LightParametersBufRef = std::make_shared<TD3DBuffer>();
	LightParametersBufRef->SRV = std::make_shared<TD3DShaderResourceView>(D3DRHI->GetDevice(), LightParametersBufRef->GpuResource.get());

	// Create LightParameter Buffer
	TD3DResourceInitInfo InitInfo = TD3DResourceInitInfo::Buffer_Upload(ElementCount * ElementSize);
	D3DRHI->GetDevice()->GetResourceAllocator()->Allocate(InitInfo, LightParametersBufRef->GpuResource.get());

	void* MappedData = nullptr;
	LightParametersBufRef->GpuResource->D3DResource->Map(0, nullptr, &MappedData);
	memcpy_s(MappedData, ElementCount * ElementSize, LightParametersArray.data(), ElementCount * ElementSize);
	LightParametersBufRef->GpuResource->D3DResource->Unmap(0, nullptr);


	TD3DViewInitInfo SRView = TD3DViewInitInfo::SRView_Buffer(LightParametersBufRef->GpuResource.get(), ElementSize, ElementCount);
	D3DRHI->GetDevice()->GetViewAllocator()->Allocate(LightParametersBufRef->SRV.get(), SRView);

	{
		TLightCommon LightCommon;
		LightCommon.LightCount = LightParametersArray.size();

		LightCommonBufRef = std::make_shared<TD3DResource>();

		TD3DResourceInitInfo InitInfo = TD3DResourceInitInfo::Buffer_Upload(sizeof(TLightCommon));
		D3DRHI->GetDevice()->GetResourceAllocator()->Allocate(InitInfo, LightCommonBufRef.get());

		void* MappedData = nullptr;
		LightCommonBufRef.get()->D3DResource->Map(0, nullptr, &MappedData);
		memcpy_s(MappedData, sizeof(TLightCommon), &LightCommon, sizeof(TLightCommon));
		LightCommonBufRef.get()->D3DResource->Unmap(0, nullptr);
	}
}
