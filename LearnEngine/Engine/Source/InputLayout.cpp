#include "InputLayout.h"
#include <d3d12.h>
#include <assert.h>

void TInputLayoutManager::AddInputLayout(const std::string& Name, const std::vector<D3D12_INPUT_ELEMENT_DESC>& InputLayout)
{
	InputLayoutMap.insert({ Name, InputLayout });
}

void TInputLayoutManager::GetInputLayout(const std::string Name, std::vector<D3D12_INPUT_ELEMENT_DESC>& OutInputLayout) const
{
	auto Iter = InputLayoutMap.find(Name);
	if (Iter == InputLayoutMap.end())
	{
		printf("TInputLayoutManager::GetInputLayout()::Not Find InputLayout::%s\n", Name.c_str());
		assert(0);
	}
	else
	{
		OutInputLayout = Iter->second;
	}
}
