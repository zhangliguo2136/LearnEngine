#pragma once

class TD3DRHI;
class TWorld;

class TRender
{
public:
	void Draw(float dt);

private:
	void BasePass();
	void DeferredLightingPass();

private:
	// D3DRHI
	TD3DRHI* D3DRHI = nullptr;
	TWorld* World = nullptr;
};