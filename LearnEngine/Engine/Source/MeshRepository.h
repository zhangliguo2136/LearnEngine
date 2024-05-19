#pragma once

#include <string>
#include <unordered_map>
#include "Mesh.h"
#include "FBXLoader.h"

class TMeshRepository
{
public:
	TMeshRepository();

	static TMeshRepository& Get();

	void Load();

	void Unload();

public:
	std::unordered_map<std::string /*MeshName*/, TMesh> MeshMap;

private:
	std::unique_ptr<TFbxLoader> FbxLoader = nullptr;
};