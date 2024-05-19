#pragma once
#include <string>
#include <memory>
#include <unordered_map>

#include "Shader.h"


class TD3DRHI;

class TMaterial
{
public:
	TMaterial(const std::string& InName, const std::string& InShaderName);

	// ���ݲ�ͬ�ĺ궨���ȡShader����
	TShader* GetShaderByDefines(const TMacroDefines& ShaderDefines, TD3DRHI* D3DRHI);

public:
	std::string Name;
	std::string ShaderName;

private:
	std::unordered_map<TMacroDefines, std::unique_ptr<TShader>> ShaderMap;
};