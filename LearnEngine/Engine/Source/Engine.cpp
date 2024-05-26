#include "Engine.h"

#include "World.h"
#include "Render.h"
#include "D3DRHI.h"
#include "WindowsApplication.h"
#include "D3DViewport.h"
#include "TextureRepository.h"
#include "MaterialRepository.h"
#include "MeshRepository.h"

TEngine::TEngine(TWorld* InWorld, TWindowsApplication* InWinApp)
{
	World.reset(InWorld);
	WinApp.reset(InWinApp);

	TTextureRepository::Get().Load();
	TMaterialRepository::Get().Load();
	TMeshRepository::Get().Load();

	TD3DViewportInfo ViewportInfo;
	ViewportInfo.WindowHandle = WinApp->GetWindowHandle();
	ViewportInfo.BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	ViewportInfo.DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	ViewportInfo.Width = WinApp->GetWindowWidth();
	ViewportInfo.Height = WinApp->GetWindowHeight();

	D3DRHI = std::make_unique<TD3DRHI>();
	D3DRHI->Initialize(ViewportInfo);

	Render = std::make_unique<TRender>();
	Render->Initialize(D3DRHI.get(), World.get());
}

void TEngine::Update(float dt)
{
	D3DRHI->BeginFrame();
	World->BeginFrame();

	World->Update(dt);
	Render->Draw(dt);

	World->EndFrame();
	D3DRHI->EndFrame();
}
