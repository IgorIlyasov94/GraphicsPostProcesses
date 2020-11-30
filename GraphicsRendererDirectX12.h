#pragma once

#include "stdafx.h"

using namespace Microsoft::WRL;

class GraphicsRendererDirectX12
{
public:
	static GraphicsRendererDirectX12& getInstance();

	ID3D12Device* const& getDevice();

	int32_t const& getResolutionX() const;
	int32_t const& getResolutionY() const;

	void initialize();
	void gpuRelease();
	void frameRender();

private:
	GraphicsRendererDirectX12();
	~GraphicsRendererDirectX12() {};

	GraphicsRendererDirectX12(const GraphicsRendererDirectX12&) = delete;
	GraphicsRendererDirectX12(GraphicsRendererDirectX12&&) = delete;
	GraphicsRendererDirectX12& operator=(const GraphicsRendererDirectX12&) = delete;
	GraphicsRendererDirectX12& operator=(GraphicsRendererDirectX12&&) = delete;

	ComPtr<ID3D12Device> device;
	ComPtr<ID3D12CommandQueue> commandQueue;
	ComPtr<ID3D12DescriptorHeap> rtvHeap;
	ComPtr<ID3D12CommandAllocator> commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> commandList;
	ComPtr<ID3D12PipelineState> pipelineState;
	ComPtr<ID3D12Fence> fence;

	//ID3D12Resource* backBufferRT[2];

	ComPtr<IDXGISwapChain3> swapChain;

	HANDLE fenceEvent;

	int32_t gpuMemory;
	int32_t bufferIndex;
	uint64_t fenceValue;

	int32_t resolutionX;
	int32_t resolutionY;

	char gpuDescription[128] = {};
};