#pragma once
#include <memory>

class TD3DDevice;
class TD3DCommandContent;
class TD3DViewport;

class TD3DRHI
{
public:
	TD3DRHI();
	~TD3DRHI();

	void Initialize();
	void Destroy();

public:
	TD3DDevice* GetDevice() { return Device.get(); }
	TD3DViewport* GetViewport() { return Viewport.get(); }
	TD3DCommandContent* GetCommandContent() { return CommandContent.get(); }

private:
	std::unique_ptr<TD3DDevice> Device = nullptr;
	std::unique_ptr<TD3DCommandContent> CommandContent = nullptr;
	std::unique_ptr<TD3DViewport> Viewport = nullptr;
};