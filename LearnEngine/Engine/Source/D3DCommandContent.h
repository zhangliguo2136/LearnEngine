#pragma once
#include <d3d12.h>
#include <wrl/module.h>
#include <memory>

#include "D3DDescriptorCache.h"

class TD3DRHI;
class TD3DDevice;

class TD3DCommandContent
{
public:
	TD3DCommandContent(TD3DRHI*, TD3DDevice*);
	~TD3DCommandContent();

private:
	void Initialize();
	void Destroy();

private:
	TD3DRHI* D3DRHI = nullptr;
	TD3DDevice* D3DDevice = nullptr;

public:
	void FlushCommandQueue();
	void ExecuteCommandList();
	void ResetCommandList();
	void ResetCommandAllocator();
public:
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList() { return CommandList; }
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue() { return CommandQueue; }

private:
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandListAllocator = nullptr;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList = nullptr;

private:
	Microsoft::WRL::ComPtr<ID3D12Fence> Fence = nullptr;
	UINT64 CurrentFenceValue = 0;

private:
	std::unique_ptr<TD3DDescriptorCache> DescriptorCache = nullptr;
public:
	TD3DDescriptorCache* GetDescriptorCache() { return DescriptorCache.get(); }
};