#include "GraphicsRendererDirectX12.h"

GraphicsRendererDirectX12::GraphicsRendererDirectX12()
	: fenceEvent(nullptr), gpuMemory(0), bufferIndex(0), swapChainRtvDescriptorSize(0),
	swapChainSrvDescriptorSize(0), fenceValues{}
{
	resolutionX = 1024;
	resolutionY = 768;

	sceneViewport.TopLeftX = 0.0f;
	sceneViewport.TopLeftY = 0.0f;
	sceneViewport.Width = static_cast<float>(resolutionX);
	sceneViewport.Height = static_cast<float>(resolutionY);
	sceneViewport.MinDepth = D3D12_MIN_DEPTH;
	sceneViewport.MaxDepth = D3D12_MAX_DEPTH;
}

GraphicsRendererDirectX12& GraphicsRendererDirectX12::GetInstance()
{
	static GraphicsRendererDirectX12 instance;

	return instance;
}

int32_t const& GraphicsRendererDirectX12::getResolutionX() const
{
	return resolutionX;
}

int32_t const& GraphicsRendererDirectX12::getResolutionY() const
{
	return resolutionY;
}

void GraphicsRendererDirectX12::Initialize(HWND& windowHandler)
{
	UINT dxgiFactoryFlags = 0;

	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

	ComPtr<IDXGIAdapter1> adapter;
	GetHardwareAdapter(factory.Get(), &adapter);

	ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));

	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue)));
	commandQueue->SetName(L"commandQueue");

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.BufferCount = SWAP_CHAIN_BUFFER_COUNT;
	swapChainDesc.Width = resolutionX;
	swapChainDesc.Height = resolutionY;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;
	
	ComPtr<IDXGISwapChain1> swapChain1;
	ThrowIfFailed(factory->CreateSwapChainForHwnd(commandQueue.Get(), windowHandler, &swapChainDesc, nullptr, nullptr, &swapChain1));

	ThrowIfFailed(swapChain1.As(&swapChain));

	bufferIndex = swapChain->GetCurrentBackBufferIndex();

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapRtvDesc{};
	descriptorHeapRtvDesc.NumDescriptors = SWAP_CHAIN_BUFFER_COUNT;
	descriptorHeapRtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descriptorHeapRtvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(device->CreateDescriptorHeap(&descriptorHeapRtvDesc, IID_PPV_ARGS(&swapChainRtvHeap)));

	swapChainRtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	swapChainSrvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	auto rtvHeapHandle(swapChainRtvHeap->GetCPUDescriptorHandleForHeapStart());

	for (auto swapChainBufferId = 0; swapChainBufferId < SWAP_CHAIN_BUFFER_COUNT; swapChainBufferId++)
	{
		ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator[swapChainBufferId])));

		ThrowIfFailed(swapChain->GetBuffer(swapChainBufferId, IID_PPV_ARGS(&swapChainBuffersRtv[swapChainBufferId])));

		device->CreateRenderTargetView(swapChainBuffersRtv[swapChainBufferId].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.ptr += swapChainRtvDescriptorSize;

		swapChainBuffersRtv[swapChainBufferId]->SetName(L"swapChainBuffersRtv");
	}

	ThrowIfFailed(device->CreateFence(fenceValues[bufferIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
	fenceValues[bufferIndex]++;

	fenceEvent = CreateEvent(nullptr, false, false, nullptr);
	if (fenceEvent == nullptr)
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));

	ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator[bufferIndex].Get(), nullptr, IID_PPV_ARGS(&commandList)));

	pipelineState = nullptr;

	postProcesses.Initialize(resolutionX, resolutionY, device.Get(), sceneViewport);

	ThrowIfFailed(commandList->Close());
}

void GraphicsRendererDirectX12::GpuRelease()
{
	WaitForGpu();

	CloseHandle(fenceEvent);
}

void GraphicsRendererDirectX12::FrameRender()
{
	ThrowIfFailed(commandAllocator[bufferIndex]->Reset());
	
	ThrowIfFailed(commandList->Reset(commandAllocator[bufferIndex].Get(), pipelineState.Get()));

	D3D12_RESOURCE_BARRIER resourceBarrier{};
	resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	resourceBarrier.Transition.pResource = swapChainBuffersRtv[bufferIndex].Get();
	resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	
	commandList->ResourceBarrier(1, &resourceBarrier);

	/*auto rtvHeapHandle(swapChainRtvHeap->GetCPUDescriptorHandleForHeapStart());
	rtvHeapHandle.ptr += bufferIndex * swapChainRtvDescriptorSize;

	commandList->OMSetRenderTargets(1, &rtvHeapHandle, false, nullptr);

	const float clearColor[] = { 0.3f, 0.6f, 0.4f, 1.0f };
	
	commandList->ClearRenderTargetView(rtvHeapHandle, clearColor, 0, nullptr);*/

	postProcesses.EnableHDR(commandList.Get(), swapChainRtvHeap.Get(), bufferIndex * swapChainRtvDescriptorSize);

	ThrowIfFailed(commandList->Close());

	ID3D12CommandList* commandLists[] = { commandList.Get() };

	commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	ThrowIfFailed(swapChain->Present(1, 0));

	PrepareNextFrame();
}

void GraphicsRendererDirectX12::GetHardwareAdapter(IDXGIFactory4* factory4, IDXGIAdapter1** adapter)
{
	*adapter = nullptr;

	ComPtr<IDXGIAdapter1> adapter1;
	ComPtr<IDXGIFactory6> factory6;

	if (SUCCEEDED(factory4->QueryInterface(IID_PPV_ARGS(&factory6))))
	{
		for (auto adapterId = 0;
			factory6->EnumAdapterByGpuPreference(adapterId, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter1)) != DXGI_ERROR_NOT_FOUND;
			adapterId++)
		{
			if (SUCCEEDED(D3D12CreateDevice(adapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
				break;
		}
	}
	else
	{
		for (auto adapterId = 0;
			factory4->EnumAdapters1(adapterId, &adapter1) != DXGI_ERROR_NOT_FOUND;
			adapterId++)
		{
			if (SUCCEEDED(D3D12CreateDevice(adapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
				break;
		}
	}

	*adapter = adapter1.Detach();
}

void GraphicsRendererDirectX12::PrepareNextFrame()
{
	auto currentFenceValue = fenceValues[bufferIndex];
	ThrowIfFailed(commandQueue->Signal(fence.Get(), currentFenceValue));

	bufferIndex = swapChain->GetCurrentBackBufferIndex();

	if (fence->GetCompletedValue() < fenceValues[bufferIndex])
	{
		ThrowIfFailed(fence->SetEventOnCompletion(fenceValues[bufferIndex], fenceEvent));
		WaitForSingleObjectEx(fenceEvent, INFINITE, false);
	}

	fenceValues[bufferIndex] = currentFenceValue + 1;
}

void GraphicsRendererDirectX12::WaitForGpu()
{
	ThrowIfFailed(commandQueue->Signal(fence.Get(), fenceValues[bufferIndex]));

	ThrowIfFailed(fence->SetEventOnCompletion(fenceValues[bufferIndex], fenceEvent));

	WaitForSingleObjectEx(fenceEvent, INFINITE, false);

	fenceValues[bufferIndex]++;
}