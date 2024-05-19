#pragma once
#include <string>
#include <unordered_map>
#include "D3DResource.h"

struct TSubmeshProxy
{
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	INT BaseVertexLocation = 0;
};

struct TMeshProxy
{
	std::string Name;

	std::shared_ptr<TD3DResource> VertexBuffer = nullptr;
	std::shared_ptr<TD3DResource> IndexBuffer = nullptr;

	UINT VertexByteStride = 0;
	UINT VertexBufferByteSize = 0;

	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
	UINT IndexBufferByteSize = 0;

	std::unordered_map<std::string, TSubmeshProxy> SubMeshs;
};