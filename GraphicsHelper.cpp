#include "GraphicsHelper.h"

void Graphics::CreateFactory(IDXGIFactory4** _factory)
{
	UINT dxgiFactoryFlags = 0;

	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(_factory)),
		"CreateFactory: Factory creating failed!");
}

void Graphics::CreateDevice(IDXGIAdapter1* adapter, ID3D12Device** _device)
{
	ThrowIfFailed(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(_device)),
		"CreateCommandQueue: Device creating failed!");
}

void Graphics::CreateCommandQueue(ID3D12Device* _device, ID3D12CommandQueue** _commandQueue)
{
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(_device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(_commandQueue)),
		"CreateCommandQueue: Command Queue creating failed!");
}

void Graphics::CreateSwapChain(IDXGIFactory4* _factory, ID3D12CommandQueue* _commandQueue, HWND& _windowHandler, const uint32_t buffersCount,
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

void Graphics::CreateDescriptorHeap(ID3D12Device* device, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS flags, D3D12_DESCRIPTOR_HEAP_TYPE type,
	ID3D12DescriptorHeap** descriptorHeap)
{
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = flags;
	descriptorHeapDesc.Type = type;

	ThrowIfFailed(device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(descriptorHeap)),
		"CreateDescriptorHeap: Descriptor Heap creating failed!");
}

void Graphics::ReadShaderConstantBuffers(const D3D12_SHADER_BYTECODE& shaderBytecode, std::set<size_t>& constantBufferIndices)
{
	auto shaderText = reinterpret_cast<const char*>(shaderBytecode.pShaderBytecode);

	std::regex constantBufferPattern(" cb\\d+ ");

	std::vector<std::string> constantBufferHLSLBinds;

	std::copy(std::cregex_token_iterator(shaderText, shaderText + shaderBytecode.BytecodeLength, constantBufferPattern),
		std::cregex_token_iterator(), std::back_inserter(constantBufferHLSLBinds));

	for (auto& hlslBind: constantBufferHLSLBinds)
	{
		size_t constantBufferIndex = 0;

		std::istringstream hlslBindStringStream(hlslBind.substr(3));

		hlslBindStringStream >> constantBufferIndex;

		constantBufferIndices.insert(constantBufferIndex);
	}

	std::stringstream str(std::string(shaderText).substr(0, 50));

	str << constantBufferHLSLBinds.size();

	throw std::exception(str.str().c_str());
}

void Graphics::CreateStandardSamplerDescs(std::vector<D3D12_STATIC_SAMPLER_DESC>& samplerDescs)
{
	D3D12_STATIC_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.ShaderRegister = 0;

	samplerDescs.push_back(samplerDesc);

	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.ShaderRegister = 1;

	samplerDescs.push_back(samplerDesc);

	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.ShaderRegister = 2;

	samplerDescs.push_back(samplerDesc);

	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.ShaderRegister = 3;

	samplerDescs.push_back(samplerDesc);

	samplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.ShaderRegister = 4;

	samplerDescs.push_back(samplerDesc);

	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.ShaderRegister = 5;

	samplerDescs.push_back(samplerDesc);
}

void Graphics::GetHardwareAdapter(IDXGIFactory4* factory4, IDXGIAdapter1** adapter)
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

void Graphics::SetupRasterizerDesc(D3D12_RASTERIZER_DESC& rasterizerDesc, D3D12_CULL_MODE cullMode) noexcept
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

void Graphics::SetupBlendDesc(D3D12_BLEND_DESC& blendDesc, bool blendOn,
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

void Graphics::SetupDepthStencilDesc(D3D12_DEPTH_STENCIL_DESC& depthStencilDesc, bool depthEnable) noexcept
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

void Graphics::SetupResourceBufferDesc(D3D12_RESOURCE_DESC& resourceDesc, uint64_t bufferSize, D3D12_RESOURCE_FLAGS resourceFlag, uint64_t alignment) noexcept
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

void Graphics::SetupResourceTextureDesc(D3D12_RESOURCE_DESC& resourceDesc, const TextureInfo& textureInfo, D3D12_RESOURCE_FLAGS resourceFlag, uint64_t alignment) noexcept
{
	resourceDesc.Dimension = textureInfo.dimension;
	resourceDesc.DepthOrArraySize = textureInfo.depth;
	resourceDesc.Alignment = 0;
	resourceDesc.Flags = resourceFlag;
	resourceDesc.Format = textureInfo.format;
	resourceDesc.Height = textureInfo.height;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.MipLevels = textureInfo.mipLevels;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Width = textureInfo.width;
}

void Graphics::SetupHeapProperties(D3D12_HEAP_PROPERTIES& heapProperties, D3D12_HEAP_TYPE heapType) noexcept
{
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.VisibleNodeMask = 1;
	heapProperties.Type = heapType;
}

void Graphics::SetResourceBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* const resource, D3D12_RESOURCE_STATES resourceBarrierStateBefore,
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

bool Graphics::CheckBoxInBox(const BoundingBox& sourceBox, const BoundingBox& destinationBox) noexcept
{
	if (sourceBox.maxCornerPoint.x <= destinationBox.maxCornerPoint.x &&
		sourceBox.minCornerPoint.x >= destinationBox.minCornerPoint.x &&
		sourceBox.maxCornerPoint.y <= destinationBox.maxCornerPoint.y &&
		sourceBox.minCornerPoint.y >= destinationBox.minCornerPoint.y &&
		sourceBox.maxCornerPoint.z <= destinationBox.maxCornerPoint.z &&
		sourceBox.minCornerPoint.z >= destinationBox.minCornerPoint.z)
		return true;

	return false;
}

bool Graphics::CheckBoxInBox(const float3& sourceBoxSize, const float3& destinationBoxSize) noexcept
{
	if (sourceBoxSize.x <= destinationBoxSize.x && sourceBoxSize.y <= destinationBoxSize.y && sourceBoxSize.z <= destinationBoxSize.z)
		return true;

	return false;
}

Graphics::BoundingBox Graphics::ExpandBoundingBox(const BoundingBox& targetBox, const BoundingBox& appendableBox) noexcept
{
	BoundingBox expandedBoundingBox = { {std::min(targetBox.minCornerPoint.x, appendableBox.minCornerPoint.x),
			std::min(targetBox.minCornerPoint.y, appendableBox.minCornerPoint.y),
			std::min(targetBox.minCornerPoint.z, appendableBox.minCornerPoint.z)},
			{std::max(targetBox.maxCornerPoint.x, appendableBox.maxCornerPoint.x),
			std::max(targetBox.maxCornerPoint.y, appendableBox.maxCornerPoint.y),
			std::max(targetBox.maxCornerPoint.z, appendableBox.maxCornerPoint.z)} };

	return expandedBoundingBox;
}

float3 Graphics::BoundingBoxSize(const BoundingBox& boundingBox)
{
	float3 size;
	XMStoreFloat3(&size, XMLoadFloat3(&boundingBox.maxCornerPoint) - XMLoadFloat3(&boundingBox.minCornerPoint));

	return size;
}

float Graphics::BoundingBoxVolume(const BoundingBox& boundingBox)
{
	float3 size = BoundingBoxSize(boundingBox);

	float volume = size.x * size.y * size.z;

	return volume;
}

void Graphics::BoundingBoxVertices(const BoundingBox& boundingBox, std::array<floatN, 8>& vertices)
{
	float3 vertex = boundingBox.maxCornerPoint;
	vertices[0] = XMLoadFloat3(&vertex);

	vertex = { boundingBox.minCornerPoint.x, boundingBox.maxCornerPoint.y, boundingBox.maxCornerPoint.z };
	vertices[1] = XMLoadFloat3(&vertex);

	vertex = { boundingBox.minCornerPoint.x, boundingBox.maxCornerPoint.y, boundingBox.minCornerPoint.z };
	vertices[2] = XMLoadFloat3(&vertex);

	vertex = { boundingBox.maxCornerPoint.x, boundingBox.maxCornerPoint.y, boundingBox.minCornerPoint.z };
	vertices[3] = XMLoadFloat3(&vertex);

	vertex = { boundingBox.maxCornerPoint.x, boundingBox.minCornerPoint.y, boundingBox.maxCornerPoint.z };
	vertices[4] = XMLoadFloat3(&vertex);

	vertex = { boundingBox.minCornerPoint.x, boundingBox.minCornerPoint.y, boundingBox.maxCornerPoint.z };
	vertices[5] = XMLoadFloat3(&vertex);

	vertex = boundingBox.minCornerPoint;
	vertices[6] = XMLoadFloat3(&vertex);

	vertex = { boundingBox.maxCornerPoint.x, boundingBox.minCornerPoint.y, boundingBox.minCornerPoint.z };
	vertices[7] = XMLoadFloat3(&vertex);
}
