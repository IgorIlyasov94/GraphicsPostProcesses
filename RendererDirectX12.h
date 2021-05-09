#pragma once

#include "SceneManager.h"
#include "PostProcesses.h"

namespace Graphics
{
	class RendererDirectX12
	{
	public:
		static RendererDirectX12& GetInstance();

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
		ComPtr<ID3D12Fence> fence;

		ComPtr<IDXGIFactory4> factory;
		ComPtr<IDXGISwapChain4> swapChain;
		
		RenderTargetId sceneRenderTargetId;
		DepthStencilId sceneDepthStencilId;

		HANDLE fenceEvent;

		D3D12_VIEWPORT sceneViewport;
		D3D12_RECT sceneScissorRect;

		uint32_t bufferIndex;
		uint64_t fenceValues[SWAP_CHAIN_BUFFER_COUNT];

		ResourceManager& resourceManager = ResourceManager::GetInstance();
		PostProcesses& postProcesses = PostProcesses::GetInstance();
		SceneManager& sceneManager = SceneManager::GetInstance();
	};
}
