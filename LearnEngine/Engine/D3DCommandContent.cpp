#include "D3DCommandContent.h"
#include "Utils.h"
#include "D3DRHI.h"
#include "D3DDevice.h"

TD3DCommandContent::TD3DCommandContent(TD3DRHI* InRHI, TD3DDevice* InDevice) :
	D3DRHI(InRHI), D3DDevice(InDevice)
{
	Initialize();
}

TD3DCommandContent::~TD3DCommandContent()
{

}

void TD3DCommandContent::Initialize()
{
	ThrowIfFailed(D3DDevice->GetD3DDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));

	// Create CommandQueue
	D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
	QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(D3DDevice->GetD3DDevice()->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(CommandQueue.GetAddressOf())));

	// Create CommandAllocator
	ThrowIfFailed(D3DDevice->GetD3DDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(CommandListAllocator.GetAddressOf())));

	// Create CommandList 
	ThrowIfFailed(D3DDevice->GetD3DDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandListAllocator.Get(), nullptr, IID_PPV_ARGS(CommandList.GetAddressOf())));

	// CommandList Close
	// Start off in a closed state. 
	// This is because the first time we refer to the command list we will Reset it,
	// and it needs to be closed before calling Reset.
	ThrowIfFailed(CommandList->Close());
}

void TD3DCommandContent::Destroy()
{
}

void TD3DCommandContent::FlushCommandQueue()
{
	CurrentFenceValue++;

	ThrowIfFailed(CommandQueue->Signal(Fence.Get(), CurrentFenceValue));

	if (Fence->GetCompletedValue() < CurrentFenceValue)
	{
		HANDLE EventHandle = CreateEvent(nullptr, false, false, nullptr);

		ThrowIfFailed(Fence->SetEventOnCompletion(CurrentFenceValue, EventHandle));

		WaitForSingleObject(EventHandle, INFINITE);
		CloseHandle(EventHandle);
	}
}

void TD3DCommandContent::ExecuteCommands()
{
	ThrowIfFailed(CommandList->Close());

	ID3D12CommandList* CmdLists[] = { CommandList.Get() };
	CommandQueue->ExecuteCommandLists(_countof(CmdLists), CmdLists);
}

void TD3DCommandContent::ResetCommandList()
{
	ThrowIfFailed(CommandList->Reset(CommandListAllocator.Get(), nullptr));
}

void TD3DCommandContent::ResetCommandAllocator()
{
	ThrowIfFailed(CommandListAllocator->Reset());
}
