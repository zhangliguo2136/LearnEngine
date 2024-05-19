#pragma once
#include <d3d12.h>
#include <stdint.h>
#include <wrl/module.h>
#include <dxgi1_4.h>

#include "Utils.h"

class TD3DRHI;

class TD3DDevice
{
public:
	TD3DDevice(TD3DRHI*, Microsoft::WRL::ComPtr<IDXGIFactory4>);
	~TD3DDevice();

	void Initialize();

	ID3D12Device* GetD3DDevice() 
	{
		return D3DDevice.Get();
	}

private:
	Microsoft::WRL::ComPtr<ID3D12Device> D3DDevice = nullptr;
	Microsoft::WRL::ComPtr<IDXGIFactory4> DXGIFactory = nullptr;

	TD3DRHI* D3DRHI = nullptr;
};