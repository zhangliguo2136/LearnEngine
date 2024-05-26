#pragma once
#include <memory>
#include <unordered_map>

#include "MeshProxy.h"
#include "D3DTexture.h"
#include "MeshBatch.h"
#include "PipelineState.h"
#include "D3DBuffer.h"

class TD3DRHI;
class TWorld;
class TGraphicsPSOManager;
class TInputLayoutManager;
class TD3DTextureLoader;

class TRender
{
public:
	bool Initialize(TD3DRHI* InD3DRHI, TWorld* InWorld);
	void Draw(float dt);

private:
	void BasePass();
	void DeferredLightingPass();
	void PostProcessPass();

private:
	// D3DRHI
	TD3DRHI* D3DRHI = nullptr;
	TWorld* World = nullptr;

private:
	const int GBufferCount = 6;
	std::unique_ptr<TD3DTexture> GBufferBaseColor = nullptr;
	std::unique_ptr<TD3DTexture> GBufferNormal = nullptr;
	std::unique_ptr<TD3DTexture> GBufferWorldPos = nullptr;
	std::unique_ptr<TD3DTexture> GBufferORM = nullptr;
	std::unique_ptr<TD3DTexture> GBufferVelocity = nullptr;
	std::unique_ptr<TD3DTexture> GBufferEmissive = nullptr;

	std::unique_ptr<TD3DTexture> ColorTexture = nullptr;

private:
	TInputLayoutManager* InputLayoutManager = nullptr;
	TGraphicsPSOManager* GraphicsPSOManager = nullptr;
	TD3DTextureLoader* D3DTextureLoader = nullptr;

	std::unique_ptr<TShader> DeferredLightingShader = nullptr;
	TGraphicsPSODescriptor DeferredLightingPSODescriptor;

	std::unique_ptr<TShader> PostProcessShader = nullptr;
	TGraphicsPSODescriptor PostProcessPSODescriptor;

	std::unique_ptr<TShader> BasePassShader = nullptr;
	TGraphicsPSODescriptor BasePassPSODescriptor;

private:
	void CreateMeshProxys();
	void CreateTextures();
	void CreateInputLayouts();
	void CreateGBuffers();
	void CreateGlobalPipelineState();
	void CreateColorTextures();

	void UpdatePassConstants();
	void UpdateLightParameters();
private:
	std::shared_ptr<TD3DResource> PassConstBufRef = nullptr;
	std::shared_ptr<TD3DBuffer> LightParametersBufRef = nullptr;
	std::shared_ptr<TD3DResource> LightCommonBufRef = nullptr;
	//std::shared_ptr<TD3DShaderResourceView> LightParametersSRVRef = nullptr;

private:
	std::vector<TMeshBatch> MeshBatchs;
	std::unordered_map<TGraphicsPSODescriptor, std::vector<TMeshCommand>> BaseMeshCommandMap;

	// Mesh
	std::unordered_map<std::string, TMeshProxy> MeshProxyMap;
	// Texture
	std::unordered_map<std::string, TD3DTextureRef> D3DTextureMap;
};