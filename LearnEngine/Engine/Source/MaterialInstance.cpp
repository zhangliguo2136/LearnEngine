#include "MaterialInstance.h"
#include "RenderProxy.h"
#include "Vector.h"
#include "D3DRHI.h"
#include "D3DResourceAllocator.h"

TMaterialInstance::TMaterialInstance(TMaterial* InParent, const std::string& InName)
	:Parent(InParent), Name(InName)
{
	TextureMap = Parent->TextureMap;
}

void TMaterialInstance::UpdateConstants(TD3DRHI* D3DRHI)
{
	TMaterialConstant MatConst;
	MatConst.EmissiveColor = TVector3f(0.f, 0.f, 0.f);
	MatConst.ShadingModel = (uint32_t)Parent->ShadingModel;

	if (MaterialConstantsRef == nullptr)
	{
		MaterialConstantsRef = std::make_shared<TD3DResource>();

		auto ResourceAllocator = D3DRHI->GetDevice()->GetResourceAllocator();

		TD3DResourceInitInfo MatConstInitInfo = TD3DResourceInitInfo::Buffer_Default(sizeof(TMaterialConstant));
		ResourceAllocator->Allocate(MatConstInitInfo, MaterialConstantsRef.get());
	}

	D3DRHI->UploadBuffer(MaterialConstantsRef.get(), &MatConst, sizeof(TMaterialConstant));
}
