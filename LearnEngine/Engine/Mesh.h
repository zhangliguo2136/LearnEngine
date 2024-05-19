#pragma once
#include <string>
#include <vector>

#include "Vertex.h"

class TMesh
{
public:
	TMesh();
	TMesh(TMesh&&) = default;
	TMesh(const TMesh&) = delete;
	TMesh& operator= (const TMesh&) = delete;

public:
	std::string MeshName;
	std::string InputLayoutName;

	std::vector<TVertex> Vertices;

	std::vector<uint32_t> Indices32;
	std::vector<uint16_t> Indices16;
};