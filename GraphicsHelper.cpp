#include "GraphicsHelper.h"

void CreateFactory(IDXGIFactory4** _factory)
{
	UINT dxgiFactoryFlags = 0;

	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(_factory)),
		"CreateFactory: Factory creating failed!");
}

void CreateDevice(IDXGIAdapter1* adapter, ID3D12Device** _device)
{
	ThrowIfFailed(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(_device)),
		"CreateCommandQueue: Device creating failed!");
}

void CreateCommandQueue(ID3D12Device* _device, ID3D12CommandQueue** _commandQueue)
{
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(_device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(_commandQueue)),
		"CreateCommandQueue: Command Queue creating failed!");
}

void CreateSwapChain(IDXGIFactory4* _factory, ID3D12CommandQueue* _commandQueue, HWND& _windowHandler, const uint32_t buffersCount,
	const int32_t& _resolutionX, const int32_t& _resolutionY, IDXGISwapChain1** _swapChain)
{
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.BufferCount = buffersCount;
	swapChainDesc.Width = _resolutionX;
	swapChainDesc.Height = _resolutionY;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ThrowIfFailed(_factory->CreateSwapChainForHwnd(_commandQueue, _windowHandler, &swapChainDesc, nullptr, nullptr, _swapChain),
		"CreateSwapChain: Swap Chain creating failed!");
}

void CreateRootSignature(ID3D12Device* device, ID3D12RootSignature** rootSignature, D3D12_ROOT_SIGNATURE_FLAGS flags)
{
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Flags = flags;

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;

	ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signature, &error),
		"CreateRootSignature: Root Signature serialization failed!");
	ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(rootSignature)),
		"CreateRootSignature: Root Signature creating failed!");
}

void CreateDescriptorHeap(ID3D12Device* device, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS flags, D3D12_DESCRIPTOR_HEAP_TYPE type,
	ID3D12DescriptorHeap** descriptorHeap)
{
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = flags;
	descriptorHeapDesc.Type = type;

	ThrowIfFailed(device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(descriptorHeap)),
		"CreateDescriptorHeap: Descriptor Heap creating failed!");
}

void CreateGraphicsPipelineState(ID3D12Device* device, D3D12_INPUT_LAYOUT_DESC&& inputLayoutDesc, ID3D12RootSignature* rootSignature,
	D3D12_RASTERIZER_DESC& rasterizerDesc, D3D12_BLEND_DESC& blendDesc, D3D12_DEPTH_STENCIL_DESC& depthStencilDesc,
	DXGI_FORMAT rtvFormat, D3D12_SHADER_BYTECODE&& vertexShader, D3D12_SHADER_BYTECODE&& pixelShader, ID3D12PipelineState** pipelineState)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{};
	pipelineStateDesc.InputLayout = inputLayoutDesc;
	pipelineStateDesc.pRootSignature = rootSignature;
	pipelineStateDesc.RasterizerState = rasterizerDesc;
	pipelineStateDesc.BlendState = blendDesc;
	pipelineStateDesc.VS = vertexShader;
	pipelineStateDesc.PS = pixelShader;
	pipelineStateDesc.DepthStencilState = depthStencilDesc;
	pipelineStateDesc.SampleMask = UINT_MAX;
	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateDesc.NumRenderTargets = 1;
	pipelineStateDesc.RTVFormats[0] = rtvFormat;
	pipelineStateDesc.SampleDesc.Count = 1;

	ThrowIfFailed(device->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(pipelineState)),
		"CreateGraphicsPipelineState: Graphics Pipeline State creating failed!");
}

/*void CreateVertexBuffer(ID3D12Device* device, uint8_t* data, uint32_t dataSize, uint32_t dataStride, D3D12_VERTEX_BUFFER_VIEW& vertexBufferView,
	ID3D12Resource** vertexBuffer, ID3D12Resource** vertexBufferUpload, ID3D12GraphicsCommandList* commandList)
{
	D3D12_HEAP_PROPERTIES heapProperties;
	SetupHeapProperties(heapProperties, D3D12_HEAP_TYPE_DEFAULT);

	D3D12_RESOURCE_DESC resourceDesc;
	SetupResourceBufferDesc(resourceDesc, dataSize);

	ThrowIfFailed(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(vertexBuffer)),"");

	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	ThrowIfFailed(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(vertexBufferUpload)),"");

	uint8_t* mappedUploadHeap = nullptr;
	D3D12_RANGE readRange = { 0, 0 };

	ThrowIfFailed((*vertexBufferUpload)->Map(0, &readRange, reinterpret_cast<void**>(&mappedUploadHeap)),"");

	std::copy(data, data + dataSize, mappedUploadHeap);
	
	(*vertexBufferUpload)->Unmap(0, &readRange);

	vertexBufferView.BufferLocation = (*vertexBuffer)->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = dataSize;
	vertexBufferView.StrideInBytes = dataStride;

	D3D12_RESOURCE_BARRIER vertexBufferResourceBarrier{};
	vertexBufferResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	vertexBufferResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	vertexBufferResourceBarrier.Transition.pResource = *vertexBuffer;
	vertexBufferResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	vertexBufferResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
	vertexBufferResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	commandList->ResourceBarrier(1, &vertexBufferResourceBarrier);
	commandList->CopyBufferRegion(*vertexBuffer, 0, *vertexBufferUpload, 0, (*vertexBuffer)->GetDesc().Width);

	vertexBufferResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	vertexBufferResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;

	commandList->ResourceBarrier(1, &vertexBufferResourceBarrier);
}*/

/*void CreateConstantBuffer(ID3D12Device* device, std::vector<uint8_t*>& data, std::vector<uint64_t>& bufferSizes, ID3D12DescriptorHeap* descriptorHeap,
	ID3D12Resource** constantBuffer, ID3D12GraphicsCommandList* commandList)
{
	if (data.size() > bufferSizes.size())
		throw std::exception("Not all constant buffers have size information");

	uint64_t constantBufferTotalSize = 0;

	for (auto& constantBufferSize : bufferSizes)
	{
		constantBufferTotalSize += constantBufferSize;
	}

	constantBufferTotalSize = 1024 * 64;
	//constantBufferTotalSize &= ~(1024 * 64);

	D3D12_HEAP_PROPERTIES heapProperties;
	SetupHeapProperties(heapProperties, D3D12_HEAP_TYPE_UPLOAD);

	D3D12_RESOURCE_DESC resourceDesc;
	SetupResourceBufferDesc(resourceDesc, constantBufferTotalSize);

	ThrowIfFailed(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(constantBuffer)),"");
	
	D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_CONSTANT_BUFFER_VIEW_DESC constantBufferViewDesc{};
	constantBufferViewDesc.BufferLocation = (*constantBuffer)->GetGPUVirtualAddress();

	uint32_t shaderResourceViewDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	
	for (auto& constantBufferSize : bufferSizes)
	{
		constantBufferViewDesc.SizeInBytes = (constantBufferSize + 255) & ~255;

		device->CreateConstantBufferView(&constantBufferViewDesc, cpuDescriptorHandle);

		constantBufferViewDesc.BufferLocation += (constantBufferSize + 255) & ~255;
		cpuDescriptorHandle.ptr += shaderResourceViewDescriptorSize;
	}
	
	uint8_t* mappedUploadHeap = nullptr;
	D3D12_RANGE readRange = { 0, 0 };

	ThrowIfFailed((*constantBuffer)->Map(0, &readRange, reinterpret_cast<void**>(&mappedUploadHeap)),"");
	
	std::copy(data.begin(), data.end(), &mappedUploadHeap);
}*/

void GetHardwareAdapter(IDXGIFactory4* factory4, IDXGIAdapter1** adapter)
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
		for (auto adapterId = 0; factory4->EnumAdapters1(adapterId, &adapter1) != DXGI_ERROR_NOT_FOUND; adapterId++)
		{
			if (SUCCEEDED(D3D12CreateDevice(adapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
				break;
		}
	}

	*adapter = adapter1.Detach();
}

void SetupRasterizerDesc(D3D12_RASTERIZER_DESC& rasterizerDesc, D3D12_CULL_MODE cullMode) noexcept
{
	rasterizerDesc.AntialiasedLineEnable = false;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	rasterizerDesc.CullMode = cullMode;
	rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
}

void SetupBlendDesc(D3D12_BLEND_DESC& blendDesc, bool blendOn,
	D3D12_BLEND srcBlend, D3D12_BLEND destBlend, D3D12_BLEND_OP blendOp,
	D3D12_BLEND srcBlendAlpha, D3D12_BLEND destBlendAlpha, D3D12_BLEND_OP blendOpAlpha) noexcept
{
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	
	const D3D12_RENDER_TARGET_BLEND_DESC rtBlendDescDefault =
	{
		false, false,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP, D3D12_COLOR_WRITE_ENABLE_ALL
	};

	for (auto renderTargetId = 0; renderTargetId < 8; renderTargetId++)
	{
		blendDesc.RenderTarget[renderTargetId] = rtBlendDescDefault;
	}

	blendDesc.RenderTarget[0].BlendEnable = blendOn;
	blendDesc.RenderTarget[0].SrcBlend = srcBlend;
	blendDesc.RenderTarget[0].DestBlend = destBlend;
	blendDesc.RenderTarget[0].BlendOp = blendOp;
	blendDesc.RenderTarget[0].SrcBlendAlpha = srcBlendAlpha;
	blendDesc.RenderTarget[0].DestBlendAlpha = destBlendAlpha;
	blendDesc.RenderTarget[0].BlendOpAlpha = blendOpAlpha;
}

void SetupDepthStencilDesc(D3D12_DEPTH_STENCIL_DESC& depthStencilDesc, bool depthEnable) noexcept
{
	depthStencilDesc.DepthEnable = depthEnable;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.StencilEnable = false;
	depthStencilDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	depthStencilDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	
	const D3D12_DEPTH_STENCILOP_DESC depthStencilOpDesc =
	{
		D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS
	};

	depthStencilDesc.BackFace = depthStencilOpDesc;
	depthStencilDesc.FrontFace = depthStencilOpDesc;
}

void SetupResourceBufferDesc(D3D12_RESOURCE_DESC& resourceDesc, uint64_t bufferSize, D3D12_RESOURCE_FLAGS resourceFlag, uint64_t alignment) noexcept
{
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Alignment = alignment;
	resourceDesc.Flags = resourceFlag;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.Height = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Width = (bufferSize + 255Ui64) & ~255Ui64;
}

void SetupHeapProperties(D3D12_HEAP_PROPERTIES& heapProperties, D3D12_HEAP_TYPE heapType) noexcept
{
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.VisibleNodeMask = 1;
	heapProperties.Type = heapType;
}

void SetResourceBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* const resource, D3D12_RESOURCE_STATES resourceBarrierStateBefore,
	D3D12_RESOURCE_STATES resourceBarrierStateAfter, D3D12_RESOURCE_BARRIER_TYPE resourceBarrierType)
{
	D3D12_RESOURCE_BARRIER resourceBarrier{};
	resourceBarrier.Type = resourceBarrierType;
	resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	resourceBarrier.Transition.pResource = resource;
	resourceBarrier.Transition.StateBefore = resourceBarrierStateBefore;
	resourceBarrier.Transition.StateAfter = resourceBarrierStateAfter;
	resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	commandList->ResourceBarrier(1, &resourceBarrier);
}
