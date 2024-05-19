#pragma once
#include <memory>
#include <unordered_map>

#include "MeshProxy.h"
#include "D3DTexture.h"
#include "MeshBatch.h"

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

private:
	TInputLayoutManager* InputLayoutManager = nullptr;
	TGraphicsPSOManager* GraphicsPSOManager = nullptr;
	TD3DTextureLoader* D3DTextureLoader = nullptr;

private:
	void CreateMeshProxys();
	void CreateTextures();
	void CreateInputLayouts();
	void CreateGBuffers();

	void UpdatePassConstants();

private:
	std::shared_ptr<TD3DResource> PassConstBufRef = nullptr;

private:
	std::vector<TMeshBatch> MeshBatchs;

	// Mesh
	std::unordered_map<std::string, TMeshProxy> MeshProxyMap;
	// Texture
	std::unordered_map<std::string, TD3DTextureRef> D3DTextureMap;
};