#include "D3DTextureLoader.h"
#include "D3DRHI.h"
#include "D3DViewAllocator.h"
#include "D3DResourceAllocator.h"

TD3DTextureLoader::TD3DTextureLoader(TD3DRHI* InD3DRHI)
{
	D3DRHI = InD3DRHI;
}

TD3DTextureLoader::~TD3DTextureLoader()
{
}

void TD3DTextureLoader::Load(TD3DTextureRef D3DTextureRef, TTextureInfo& TextureInfo)
{
	auto ResourceAllocator = D3DRHI->GetDevice()->GetResourceAllocator();
	auto ViewAllocator = D3DRHI->GetDevice()->GetViewAllocator();

	// 通过通道数和通道大小获取数据格式
	DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	if (TextureInfo.ComponentSize == 4)
	{
		Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	}

	// 创建D3DTexture;
	TD3DResourceInitInfo InitInfo = TD3DResourceInitInfo::Texture2D(TextureInfo.Width, TextureInfo.Height, Format);
	ResourceAllocator->Allocate(InitInfo, D3DTextureRef->GpuResource.get());

	// 创建SRV
	//D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc;
	//SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//SrvDesc.Format = Format;
	//SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	//SrvDesc.Texture2D.MostDetailedMip = 0;
	//SrvDesc.Texture2D.MipLevels = 1;
	//SrvDesc.Texture2D.PlaneSlice = 0;

	//D3DTextureRef->SRV = std::make_shared<TD3DShaderResourceView>(D3DRHI->GetDevice(), D3DTextureRef->GpuResource.get());
	//ViewAllocator->CreateShaderResourceView(D3DTextureRef->GpuResource.get(), D3DTextureRef->SRV.get(), SrvDesc);

	D3DTextureRef->SRV = std::make_shared<TD3DShaderResourceView>(D3DRHI->GetDevice(), D3DTextureRef->GpuResource.get());

	TD3DViewInitInfo SRViewInfo = TD3DViewInitInfo::SRView_Texture2D(D3DTextureRef->GpuResource.get(), Format);
	ViewAllocator->Allocate(D3DTextureRef->SRV.get(), SRViewInfo);

	// 上传到Default Heap
	D3DRHI->UploadTexture(D3DTextureRef->GpuResource.get(), TextureInfo);

}
