#pragma once

#include "PostProcesses.h"

namespace Graphics
{
	class RendererDirectX12
	{
	public:
		static RendererDirectX12& GetInstance();

		int32_t const& getResolutionX() const;
		int32_t const& getResolutionY() const;

		void Initialize(HWND& windowHandler);
		void GpuRelease();
		void FrameRender();

	private:
		RendererDirectX12();
		~RendererDirectX12() {}

		RendererDirectX12(const RendererDirectX12&) = delete;
		RendererDirectX12(RendererDirectX12&&) = delete;
		RendererDirectX12& operator=(const RendererDirectX12&) = delete;
		RendererDirectX12& operator=(RendererDirectX12&&) = delete;

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

		ResourceManager& resourceManager = ResourceManager::GetInstance();
		PostProcesses& postProcesses = PostProcesses::GetInstance();
	};
}
