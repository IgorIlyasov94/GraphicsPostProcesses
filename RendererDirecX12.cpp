#include "RendererDirectX12.h"

Graphics::RendererDirectX12::RendererDirectX12()
	: fenceEvent(nullptr), bufferIndex(0), swapChainRtvDescriptorSize(0), fenceValues{}
{
	sceneViewport.TopLeftX = 0.0f;
	sceneViewport.TopLeftY = 0.0f;
	sceneViewport.Width = static_cast<float>(GraphicsSettings::GetResolutionX());
	sceneViewport.Height = static_cast<float>(GraphicsSettings::GetResolutionY());
	sceneViewport.MinDepth = D3D12_MIN_DEPTH;
	sceneViewport.MaxDepth = D3D12_MAX_DEPTH;
}

Graphics::RendererDirectX12& Graphics::RendererDirectX12::GetInstance()
{
	static RendererDirectX12 instance;

	return instance;
}

void Graphics::RendererDirectX12::Initialize(HWND& windowHandler)
{
#if defined(_DEBUG)
	EnableDebugLayer();
#endif

	CreateFactory(&factory);

	ComPtr<IDXGIAdapter1> adapter;
	GetHardwareAdapter(factory.Get(), &adapter);

	CreateDevice(adapter.Get(), &device);

	CreateCommandQueue(device.Get(), &commandQueue);
	
	ComPtr<IDXGISwapChain1> swapChain1;
	CreateSwapChain(factory.Get(), commandQueue.Get(), windowHandler, SWAP_CHAIN_BUFFER_COUNT, GraphicsSettings::GetResolutionX(), GraphicsSettings::GetResolutionY(),
		&swapChain1);
	
	ThrowIfFailed(swapChain1.As(&swapChain), "RendererDirectX12::Initialize: Swap Chain conversion error!");

	bufferIndex = swapChain->GetCurrentBackBufferIndex();

	for (auto swapChainBufferId = 0; swapChainBufferId < SWAP_CHAIN_BUFFER_COUNT; swapChainBufferId++)
		ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator[swapChainBufferId])),
			"RendererDirectX12::Initialize: Command Allocator creating error!");

	ThrowIfFailed(device->CreateFence(fenceValues[bufferIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)),
		"RendererDirectX12::Initialize: Fence creating error!");
	fenceValues[bufferIndex]++;

	fenceEvent = CreateEvent(nullptr, false, false, nullptr);
	if (fenceEvent == nullptr)
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()), "RendererDirectX12::Initialize: Fence Event creating error!");

	ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator[bufferIndex].Get(), nullptr, IID_PPV_ARGS(&commandList)),
		"RendererDirectX12::Initialize: Command List creating error!");

	pipelineState = nullptr;

	resourceManager.Initialize(device.Get(), commandList.Get());
	resourceManager.CreateSwapChainBuffers(swapChain.Get(), SWAP_CHAIN_BUFFER_COUNT);

	postProcesses.Initialize(GraphicsSettings::GetResolutionX(), GraphicsSettings::GetResolutionY(), device.Get(), sceneViewport, commandList.Get());

	ThrowIfFailed(commandList->Close(), "RendererDirectX12::Initialize: Command List closing error!");

	ID3D12CommandList* commandLists[] = { commandList.Get() };

	commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	WaitForGpu();

	resourceManager.ReleaseTemporaryUploadBuffers();
}

void Graphics::RendererDirectX12::GpuRelease()
{
	WaitForGpu();

	CloseHandle(fenceEvent);
}

void Graphics::RendererDirectX12::FrameRender()
{
	ThrowIfFailed(commandAllocator[bufferIndex]->Reset(), "RendererDirectX12::FrameRender: Command Allocator resetting error!");
	ThrowIfFailed(commandList->Reset(commandAllocator[bufferIndex].Get(), pipelineState.Get()), "RendererDirectX12::FrameRender: Command List resetting error!");

	SetResourceBarrier(commandList.Get(), resourceManager.GetSwapChainBuffer(bufferIndex), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	commandList->OMSetRenderTargets(1, &resourceManager.GetSwapChainDescriptorBase(bufferIndex), false, nullptr);

	const float clearColor[] = { 0.3f, 0.6f, 0.4f, 1.0f };
	commandList->ClearRenderTargetView(resourceManager.GetSwapChainDescriptorBase(bufferIndex), clearColor, 0, nullptr);

	sceneManager.DrawCurrentScene(commandList.Get());

	postProcesses.EnableHDR(commandList.Get(), bufferIndex);
	
	SetResourceBarrier(commandList.Get(), resourceManager.GetSwapChainBuffer(bufferIndex), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	ThrowIfFailed(commandList->Close(), "RendererDirectX12::FrameRender: Command List closing error!");

	ID3D12CommandList* commandLists[] = { commandList.Get() };

	commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	ThrowIfFailed(swapChain->Present(1, 0), "RendererDirectX12::FrameRender: Frame not presented!");

	PrepareNextFrame();
}

void Graphics::RendererDirectX12::PrepareNextFrame()
{
	auto currentFenceValue = fenceValues[bufferIndex];
	ThrowIfFailed(commandQueue->Signal(fence.Get(), currentFenceValue), "RendererDirectX12::PrepareNextFrame: Signal error!");

	bufferIndex = swapChain->GetCurrentBackBufferIndex();

	if (fence->GetCompletedValue() < fenceValues[bufferIndex])
	{
		ThrowIfFailed(fence->SetEventOnCompletion(fenceValues[bufferIndex], fenceEvent), "RendererDirectX12::PrepareNextFrame: Fence error!");
		WaitForSingleObjectEx(fenceEvent, INFINITE, false);
	}

	fenceValues[bufferIndex] = currentFenceValue + 1;
}

void Graphics::RendererDirectX12::WaitForGpu()
{
	ThrowIfFailed(commandQueue->Signal(fence.Get(), fenceValues[bufferIndex]), "RendererDirectX12::WaitForGpu: Signal error!");
	ThrowIfFailed(fence->SetEventOnCompletion(fenceValues[bufferIndex], fenceEvent), "RendererDirectX12::WaitForGpu: Fence error!");

	WaitForSingleObjectEx(fenceEvent, INFINITE, false);

	fenceValues[bufferIndex]++;
}

#if defined(_DEBUG)
void Graphics::RendererDirectX12::EnableDebugLayer()
{
	ComPtr<ID3D12Debug> debugInterface;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)), "RendererDirectX12::EnableDebugLayer: Debug interface getting error!");
	debugInterface->EnableDebugLayer();
}
#endif