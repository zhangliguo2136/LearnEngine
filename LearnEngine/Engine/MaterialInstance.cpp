#include "MaterialInstance.h"

TMaterialInstance::TMaterialInstance(TMaterial* InParent, const std::string& InName)
	:Parent(InParent), Name(InName)
{
}
