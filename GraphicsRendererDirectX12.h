#pragma once

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <d3d12.h>
#include <dxgi1_6.h>

#include <cstdint>

class GraphicsRendererDirectX12
{
public:


private:
	ID3D12Device* device;
	ID3D12CommandQueue* commandQueue;
	ID3D12DescriptorHeap* rtvHeap;
	ID3D12CommandAllocator* commandAllocator;
	ID3D12GraphicsCommandList* commandList;
	ID3D12PipelineState* pipelineState;
	ID3D12Fence* fence;

	ID3D12Resource* backBufferRT[2];

	IDXGISwapChain3* swapChain;

	HANDLE fenceEvent;

	int32_t GPUMemory;
	int32_t bufferIndex;
	uint64_t fenceValue;

	char GPUDescription[128];
};