#pragma once
#include <string>
#include <unordered_map>
#include <d3d12.h>

class TInputLayoutManager
{
public:
	void AddInputLayout(const std::string& Name, const std::vector<D3D12_INPUT_ELEMENT_DESC>& InputLayout);

	void GetInputLayout(const std::string Name, std::vector<D3D12_INPUT_ELEMENT_DESC>& OutInputLayout) const;

private:
	std::unordered_map<std::string/*Name*/, std::vector<D3D12_INPUT_ELEMENT_DESC>> InputLayoutMap;
};