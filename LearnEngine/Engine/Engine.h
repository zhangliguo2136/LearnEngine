#pragma once
#include <memory>

class TD3DRHI;
class TWorld;
class TRender;

class TEngine
{
public:
	TEngine(TWorld* World);

public:
	void Update(float dt);

private:
	std::unique_ptr<TD3DRHI> D3DRHI = nullptr;
	std::unique_ptr<TWorld> World = nullptr;
	std::unique_ptr<TRender> Render = nullptr;
};