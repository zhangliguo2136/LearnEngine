#include <iostream>

#include "WindowsApplication.h"
#include "World.h"
#include "Engine.h"
#include "DirectionalLightActor.h"
#include "SkyActor.h"
#include "StaticMeshActor.h"

class TSampleWorld :public TWorld
{
public:
	TSampleWorld() {}

	void InitWorld()
	{
		 // Add DirectionalLight
		{
			auto Light = AddActor<TDirectionalLightActor>("DirectionalLight");
			TTransform Transform;
			Transform.Location = TVector3f(0.0f, 10.0f, 0.0f);
			Transform.Rotation = TRotator(0.0f, -90.0f, 0.0f);
			Light->SetActorTransform(Transform);
			Light->SetLightColor({ 1.0f, 1.0f, 1.0f });
			Light->SetLightIntensity(5.0f);
		}

		// Add Sky 
		{
			auto Sky = AddActor<TSkyActor>("Sky");
			Sky->SetMaterialInstance("SkyMat_Inst");
			TTransform Transform;
			Transform.Scale = TVector3f(5000.0f, 5000.0f, 5000.0f);
			Sky->SetActorTransform(Transform);
		}

		// Add CyborgWeapon
		{
			auto CyborgWeapon = AddActor<TStaticMeshActor>("CyborgWeapon");
			CyborgWeapon->SetMeshName("CyborgWeapon");
			CyborgWeapon->SetMaterialInstance("CyborgWeaponMat_Inst");
			TTransform Transform;
			Transform.Location = TVector3f(-2.0f, 1.0f, -5.0f);
			Transform.Rotation = TRotator(0.0f, 0.0f, -45.0f);
			Transform.Scale = TVector3f(300.0f, 300.0f, 300.0f);
			CyborgWeapon->SetActorTransform(Transform);
		}
	}

	~TSampleWorld() {}
};

int main()
{
	printf("Enter Sample World!\n");

	TSampleWorld* World = new TSampleWorld(); 

	TWindowsApplication* App = new TWindowsApplication();
	App->Initialize();

	TEngine* Engine = new TEngine(World, App);

	World->InitWorld();

	while (!App->IsQuit())
	{
		App->Tick(Engine);
	}
	App->Finalize();

	return 0;
}