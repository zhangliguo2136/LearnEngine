#pragma once

#include <memory>

#include "D3DResource.h"
#include "D3DView.h"

class TD3DTexture
{
//public:
//	void SetSRV(std::unique_ptr<TD3D12ShaderResourceView>& InSRV)
//	{
//		SRV = std::move(InSRV);
//	}
//	TD3D12ShaderResourceView* GetSRV()
//	{
//		return SRV.get();
//	}
//
//	void SetUAV(std::unique_ptr<TD3D12UnorderedAccessView>& InUAV)
//	{
//		UAV = std::move(InUAV);
//	}
//	TD3D12UnorderedAccessView* GetUAV()
//	{
//		return UAV.get();
//	}
//
//	void SetRTV(std::unique_ptr<TD3D12RenderTargetView>& InRTV)
//	{
//		RTV = std::move(InRTV);
//	}
//	TD3D12RenderTargetView* GetRTV()
//	{
//		return RTV.get();
//	}
//
//	void SetDSV(std::unique_ptr<TD3D12DepthStencilView>& InDSV)
//	{
//		DSV = std::move(InDSV);
//	}
//	TD3D12DepthStencilView* GetDSV()
//	{
//		return DSV.get();
//	}
//
public:

	// 描述符描述的是这个资源的用途
	std::unique_ptr<TD3DShaderResourceView> SRV = nullptr;
	std::unique_ptr<TD3DUnorderedAccessView> UAV = nullptr;
	std::unique_ptr<TD3DRenderTargetView> RTV = nullptr;
	std::unique_ptr<TD3DDepthStencilView> DSV = nullptr;

public:
	TD3DResource GpuResource;
};

typedef std::shared_ptr<TD3DTexture> TD3DTextureRef;
