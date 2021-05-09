#include "RendererDirectX12.h"

Graphics::RendererDirectX12::RendererDirectX12()
	: fenceEvent(nullptr), bufferIndex(0), sceneDepthStencilId{}, fenceValues{}
{
	sceneViewport.TopLeftX = 0.0f;
	sceneViewport.TopLeftY = 0.0f;
	sceneViewport.Width = static_cast<float>(GraphicsSettings::GetResolutionX());
	sceneViewport.Height = static_cast<float>(GraphicsSettings::GetResolutionY());
	sceneViewport.MinDepth = D3D12_MIN_DEPTH;
	sceneViewport.MaxDepth = D3D12_MAX_DEPTH;

	sceneScissorRect.left = 0;
	sceneScissorRect.top = 0;
	sceneScissorRect.right = GraphicsSettings::GetResolutionX();
	sceneScissorRect.bottom = GraphicsSettings::GetResolutionY();
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

	resourceManager.Initialize(device.Get(), commandList.Get());
	resourceManager.CreateSwapChainBuffers(swapChain.Get(), SWAP_CHAIN_BUFFER_COUNT);

	sceneRenderTargetId = resourceManager.CreateRenderTarget(GraphicsSettings::GetResolutionX(), GraphicsSettings::GetResolutionY(), DXGI_FORMAT_R8G8B8A8_UNORM);
	sceneDepthStencilId = resourceManager.CreateDepthStencil(GraphicsSettings::GetResolutionX(), GraphicsSettings::GetResolutionY(), 32);

	//Test
	sceneManager.InitializeTestScene(device.Get());

	postProcesses.Initialize(device.Get(), commandList.Get(), GraphicsSettings::GetResolutionX(), GraphicsSettings::GetResolutionY(), sceneViewport, sceneScissorRect,
		sceneRenderTargetId);
	postProcesses.EnableHDR(float3(0.01f, 0.01f, 0.01f), 0.6f, 0.8f, 5.0f, 10.0f);

	postProcesses.Compose(device.Get(), commandList.Get());

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
	ThrowIfFailed(commandList->Reset(commandAllocator[bufferIndex].Get(), nullptr), "RendererDirectX12::FrameRender: Command List resetting error!");

	SetResourceBarrier(commandList.Get(), resourceManager.GetSwapChainBuffer(bufferIndex), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	commandList->RSSetViewports(1, &sceneViewport);
	commandList->RSSetScissorRects(1, &sceneScissorRect);
	
	ID3D12DescriptorHeap* descHeaps[] = { resourceManager.GetShaderResourceViewDescriptorHeap() };
	commandList->SetDescriptorHeaps(_countof(descHeaps), descHeaps);

	commandList->OMSetRenderTargets(1, &resourceManager.GetRenderTargetDescriptorBase(sceneRenderTargetId), true,
		&resourceManager.GetDepthStencilDescriptorBase(sceneDepthStencilId));

	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	commandList->ClearRenderTargetView(resourceManager.GetRenderTargetDescriptorBase(sceneRenderTargetId), clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(resourceManager.GetDepthStencilDescriptorBase(sceneDepthStencilId), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	sceneManager.DrawCurrentScene(commandList.Get());

	SetResourceBarrier(commandList.Get(), resourceManager.GetRenderTarget(sceneRenderTargetId).textureAllocation.textureResource, D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	postProcesses.PresentProcessChain(commandList.Get(), &resourceManager.GetSwapChainDescriptorBase(bufferIndex));

	SetResourceBarrier(commandList.Get(), resourceManager.GetRenderTarget(sceneRenderTargetId).textureAllocation.textureResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET);

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