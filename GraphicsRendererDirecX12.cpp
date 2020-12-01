#include "GraphicsRendererDirectX12.h"

GraphicsRendererDirectX12::GraphicsRendererDirectX12()
	: fenceEvent(nullptr), gpuMemory(0), bufferIndex(0), resolutionX(1024),
	resolutionY(768), swapChainRtvDescriptorSize(0), swapChainSrvDescriptorSize(0),
	fenceValues{}
{

}

GraphicsRendererDirectX12& GraphicsRendererDirectX12::getInstance()
{
	static GraphicsRendererDirectX12 instance;

	return instance;
}

ID3D12Device* const& GraphicsRendererDirectX12::getDevice()
{
	return device.Get();
}

int32_t const& GraphicsRendererDirectX12::getResolutionX() const
{
	return resolutionX;
}

int32_t const& GraphicsRendererDirectX12::getResolutionY() const
{
	return resolutionY;
}

void GraphicsRendererDirectX12::initialize(HWND& windowHandler)
{
	UINT dxgiFactoryFlags = 0;

	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

	ComPtr<IDXGIAdapter1> adapter;
	getHardwareAdapter(factory.Get(), &adapter);

	ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));

	auto commandQueueDesc = D3D12_COMMAND_QUEUE_DESC({});
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue)));
	commandQueue->SetName(L"commandQueue");

	auto swapChainDesc = DXGI_SWAP_CHAIN_DESC1({});
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

	auto descriptorHeapRtvDesc = D3D12_DESCRIPTOR_HEAP_DESC({});
	descriptorHeapRtvDesc.NumDescriptors = SWAP_CHAIN_BUFFER_COUNT + 2;
	descriptorHeapRtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descriptorHeapRtvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(device->CreateDescriptorHeap(&descriptorHeapRtvDesc, IID_PPV_ARGS(&swapChainRtvHeap)));

	auto descriptorHeapSrvDesc = D3D12_DESCRIPTOR_HEAP_DESC({});
	descriptorHeapSrvDesc.NumDescriptors = 2;
	descriptorHeapSrvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapSrvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(device->CreateDescriptorHeap(&descriptorHeapSrvDesc, IID_PPV_ARGS(&swapChainSrvHeap)));

	swapChainRtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	swapChainSrvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	for (auto swapChainBufferId = 0; swapChainBufferId < SWAP_CHAIN_BUFFER_COUNT; swapChainBufferId++)
	{
		ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator[swapChainBufferId])));
	}

	ThrowIfFailed(device->CreateFence(fenceValues[bufferIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
	fenceValues[bufferIndex]++;

	fenceEvent = CreateEvent(nullptr, false, false, nullptr);
	if (fenceEvent == nullptr)
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
}

void GraphicsRendererDirectX12::gpuRelease()
{
	waitForGpu();

	CloseHandle(fenceEvent);
}

void GraphicsRendererDirectX12::frameRender()
{
	ThrowIfFailed(swapChain->Present(1, 0));

	prepareNextFrame();
}

void GraphicsRendererDirectX12::getHardwareAdapter(IDXGIFactory4* factory4, IDXGIAdapter1** adapter)
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

void GraphicsRendererDirectX12::prepareNextFrame()
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

void GraphicsRendererDirectX12::waitForGpu()
{
	ThrowIfFailed(commandQueue->Signal(fence.Get(), fenceValues[bufferIndex]));

	ThrowIfFailed(fence->SetEventOnCompletion(fenceValues[bufferIndex], fenceEvent));

	WaitForSingleObjectEx(fenceEvent, INFINITE, false);

	fenceValues[bufferIndex]++;
}