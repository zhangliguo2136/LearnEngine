#pragma once
#include <memory>
#include "Texture.h"

class TD3DDevice;
class TD3DCommandContent;
class TD3DViewport;
struct TD3DViewportInfo;

class TD3DRHI
{
public:
	TD3DRHI();
	~TD3DRHI();

	void Initialize(TD3DViewportInfo ViewportInfo);
	void Destroy();

	void BeginFrame();
	void EndFrame();

public:
	TD3DDevice* GetDevice() { return Device.get(); }
	TD3DViewport* GetViewport() { return Viewport.get(); }
	TD3DCommandContent* GetCommandContent() { return CommandContent.get(); }

public:
	void UploadBuffer(TD3DResource* DestBuffer, void* Content, uint32_t Size);
	void UploadTexture(TD3DResource* DestTexture, TTextureInfo& SrcTextureInfo);
	void TransitionResource(TD3DResource* Resource, D3D12_RESOURCE_STATES StateAfter);

private:
	std::unique_ptr<TD3DDevice> Device = nullptr;
	std::unique_ptr<TD3DCommandContent> CommandContent = nullptr;
	std::unique_ptr<TD3DViewport> Viewport = nullptr;

private:
	std::vector<std::shared_ptr<TD3DResource>> TempUploadBuffers;
};