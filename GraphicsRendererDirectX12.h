#pragma once

#include "GraphicsPostProcesses.h"

using namespace Microsoft::WRL;

class GraphicsRendererDirectX12
{
public:
	static GraphicsRendererDirectX12& GetInstance();

	int32_t const& getResolutionX() const;
	int32_t const& getResolutionY() const;

	void Initialize(HWND& windowHandler);
	void GpuRelease();
	void FrameRender();

private:
	GraphicsRendererDirectX12();
	~GraphicsRendererDirectX12() {}

	GraphicsRendererDirectX12(const GraphicsRendererDirectX12&) = delete;
	GraphicsRendererDirectX12(GraphicsRendererDirectX12&&) = delete;
	GraphicsRendererDirectX12& operator=(const GraphicsRendererDirectX12&) = delete;
	GraphicsRendererDirectX12& operator=(GraphicsRendererDirectX12&&) = delete;

	void PrepareNextFrame();
	void WaitForGpu();

#if defined(_DEBUG)
	void EnableDebugLayer();
#endif

	static const int32_t SWAP_CHAIN_BUFFER_COUNT = 2;

	ComPtr<ID3D12Device> device;
	ComPtr<ID3D12CommandQueue> commandQueue;
	ComPtr<ID3D12DescriptorHeap> swapChainRtvHeap;
	ComPtr<ID3D12CommandAllocator> commandAllocator[SWAP_CHAIN_BUFFER_COUNT];
	ComPtr<ID3D12GraphicsCommandList> commandList;
	ComPtr<ID3D12PipelineState> pipelineState;
	ComPtr<ID3D12Fence> fence;

	ComPtr<ID3D12Resource> swapChainBuffersRtv[SWAP_CHAIN_BUFFER_COUNT];

	ComPtr<IDXGIFactory4> factory;
	ComPtr<IDXGISwapChain4> swapChain;

	HANDLE fenceEvent;

	D3D12_VIEWPORT sceneViewport;

	int32_t swapChainRtvDescriptorSize;
	int32_t swapChainSrvDescriptorSize;

	int32_t gpuMemory;
	uint32_t bufferIndex;
	uint64_t fenceValues[SWAP_CHAIN_BUFFER_COUNT];

	int32_t resolutionX;
	int32_t resolutionY;

	GraphicsResourceManager& resourceManager = GraphicsResourceManager::GetInstance();
	GraphicsPostProcesses& postProcesses = GraphicsPostProcesses::GetInstance();
};