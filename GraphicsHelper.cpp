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

void Graphics::SetResourceBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* const resource, D3D12_RESOURCE_STATES resourceBarrierStateAfter)
{
	D3D12_RESOURCE_BARRIER resourceBarrier{};
	resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
	resourceBarrier.Transition.pResource = resource;
	resourceBarrier.Transition.StateAfter = resourceBarrierStateAfter;
	resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	commandList->ResourceBarrier(1, &resourceBarrier);
}

void Graphics::SetUAVBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* const resource)
{
	D3D12_RESOURCE_BARRIER resourceBarrier{};
	resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	resourceBarrier.UAV.pResource = resource;

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

void Graphics::TriangulateFace(VertexFormat vertexFormat, const std::vector<float3>& positions, std::vector<uint32_t>& face)
{
	size_t faceStride = (vertexFormat == VertexFormat::POSITION) ? 1 : (vertexFormat == VertexFormat::POSITION_NORMAL_TEXCOORD) ? 3 : 2;
	uint32_t verticesCount = face.size() / faceStride;

	if (verticesCount == 3)
		return;

	std::vector<uint32_t> newFace;

	if (verticesCount < 3)
	{
		face = newFace;
		return;
	}

	std::list<uint32_t> freeVertices;

	for (uint32_t vertexId = 0; vertexId < verticesCount; vertexId++)
		freeVertices.push_back(vertexId);

	float3 faceNormal = CalculatePolygonNormal(vertexFormat, positions, face);
	size_t vertexItShift = 0;

	while (freeVertices.size() > 2)
	{
		auto vertexItPrevious = freeVertices.begin();
		auto vertexItCurrent = freeVertices.begin();
		auto vertexItNext = freeVertices.begin();

		if ((vertexItShift + 2) < freeVertices.size())
		{
			std::advance(vertexItPrevious, vertexItShift);
			std::advance(vertexItCurrent, vertexItShift + 1);
			std::advance(vertexItNext, vertexItShift + 2);
		}
		else
		{
			std::advance(vertexItCurrent, 2);
			vertexItNext++;
		}

		bool triangleIsIncorrect = false;

		const float3& positionPrevious = positions[face[*vertexItPrevious * faceStride]];
		const float3& positionCurrent = positions[face[*vertexItCurrent * faceStride]];
		const float3& positionNext = positions[face[*vertexItNext * faceStride]];

		for (auto& vertexId : freeVertices)
			if (vertexId != *vertexItPrevious && vertexId != *vertexItCurrent && vertexId != *vertexItNext)
				if (CheckPointInTriangle(positionPrevious, positionCurrent, positionNext, positions[face[vertexId * faceStride]]))
				{
					triangleIsIncorrect = true;

					break;
				}

		if (!triangleIsIncorrect)
			if (!CheckTriangleInPolygon(positionPrevious, positionCurrent, positionNext, faceNormal))
				triangleIsIncorrect = true;

		if (triangleIsIncorrect)
			vertexItShift++;
		else
		{
			for (uint32_t faceAttributeId = 0; faceAttributeId < faceStride; faceAttributeId++)
				newFace.push_back(face[*vertexItPrevious * faceStride + faceAttributeId]);
			for (uint32_t faceAttributeId = 0; faceAttributeId < faceStride; faceAttributeId++)
				newFace.push_back(face[*vertexItCurrent * faceStride + faceAttributeId]);
			for (uint32_t faceAttributeId = 0; faceAttributeId < faceStride; faceAttributeId++)
				newFace.push_back(face[*vertexItNext * faceStride + faceAttributeId]);

			freeVertices.erase(vertexItCurrent);
		}

		if ((vertexItShift + 1) >= freeVertices.size())
			vertexItShift = 0;
	}

	face = newFace;
}

float3 Graphics::CalculatePolygonCenter(VertexFormat vertexFormat, const std::vector<float3>& positions, const std::vector<uint32_t>& face)
{
	uint32_t faceStride = (vertexFormat == VertexFormat::POSITION) ? 1 : (vertexFormat == VertexFormat::POSITION_NORMAL_TEXCOORD) ? 3 : 2;
	uint32_t verticesCount = face.size() / faceStride;

	floatN positionSum{};

	for (uint32_t vertexId = 0; vertexId < verticesCount; vertexId += faceStride)
	{
		positionSum += XMLoadFloat3(&positions[face[vertexId]]);
	}

	float3 result{};

	XMStoreFloat3(&result, positionSum / static_cast<float>(verticesCount));

	return result;
}

float3 Graphics::CalculatePolygonNormal(VertexFormat vertexFormat, const std::vector<float3>& positions, const std::vector<uint32_t>& face)
{
	size_t faceStride = (vertexFormat == VertexFormat::POSITION) ? 1 : (vertexFormat == VertexFormat::POSITION_NORMAL_TEXCOORD) ? 3 : 2;
	uint32_t verticesCount = face.size() / faceStride;

	floatN normal{};

	float3 positionCenter = CalculatePolygonCenter(vertexFormat, positions, face);

	for (uint32_t vertexId = 0; vertexId < verticesCount; vertexId++)
	{
		const float3& positionCurrent = positions[face[vertexId * faceStride]];
		const float3& positionNext = positions[face[(vertexId * faceStride + faceStride) % (verticesCount * faceStride)]];

		floatN vector0 = XMLoadFloat3(&positionCurrent) - XMLoadFloat3(&positionCenter);
		floatN vector1 = XMLoadFloat3(&positionNext) - XMLoadFloat3(&positionCenter);

		normal += XMVector3Cross(vector0, vector1);
	}

	normal = XMVector3Normalize(normal);

	float3 result{};

	XMStoreFloat3(&result, normal);

	return result;
}

float Graphics::CalculateTriangleArea(float3 position0, float3 position1, float3 position2)
{
	floatN vector0_1 = XMLoadFloat3(&position1) - XMLoadFloat3(&position0);
	floatN vector1_2 = XMLoadFloat3(&position2) - XMLoadFloat3(&position1);

	floatN triangleVectorArea = XMVector3Cross(vector0_1, vector1_2);

	float result{};

	XMStoreFloat(&result, XMVector3Dot(triangleVectorArea, triangleVectorArea));

	return std::sqrt(result);
}

float3 Graphics::CalculateBarycentric(float3 position0, float3 position1, float3 position2, float3 point)
{
	float triangleArea = CalculateTriangleArea(position0, position1, position2);

	float3 result{};
	result.x = CalculateTriangleArea(position1, position2, point) / triangleArea;
	result.y = CalculateTriangleArea(position2, position0, point) / triangleArea;
	result.z = CalculateTriangleArea(position0, position1, point) / triangleArea;

	return result;
}

bool Graphics::CheckPointInTriangle(float3 position0, float3 position1, float3 position2, float3 point)
{
	float3 barycentric = CalculateBarycentric(position0, position1, position2, point);

	if (barycentric.x + barycentric.y + barycentric.z <= 1.0f)
		return true;

	return false;
}

bool Graphics::CheckTriangleInPolygon(float3 position0, float3 position1, float3 position2, float3 polygonNormal)
{
	float3 triangleNormal = CalculateNormal(position0, position1, position2);

	float dotNN{};
	XMStoreFloat(&dotNN, XMVector3Dot(XMLoadFloat3(&triangleNormal), XMLoadFloat3(&polygonNormal)));

	return dotNN > 0.0f;
}

float3 Graphics::CalculateNormal(float3 position0, float3 position1, float3 position2)
{
	floatN vector0_1 = XMLoadFloat3(&position1) - XMLoadFloat3(&position0);
	floatN vector1_2 = XMLoadFloat3(&position2) - XMLoadFloat3(&position0);

	floatN normal = XMVector3Cross(vector0_1, vector1_2);
	normal = XMVector3Normalize(normal);

	float3 result;

	XMStoreFloat3(&result, normal);

	return result;
}

void Graphics::CalculateNormals(VertexFormat vertexFormat, const std::vector<float3>& positions, std::vector<uint32_t>& faces, std::vector<float3>& normals)
{
	std::vector<uint32_t> newFaces;
	std::vector<float3> newNormals;

	size_t faceStride = (vertexFormat == VertexFormat::POSITION) ? 1 : (vertexFormat == VertexFormat::POSITION_NORMAL_TEXCOORD) ? 3 : 2;
	size_t trianglesCount = faces.size() / (3 * faceStride);

	for (size_t triangleId = 0; triangleId < trianglesCount; triangleId++)
	{
		std::array<uint32_t, 3> positionIndex{};
		std::array<uint32_t, 3> texCoordIndex{};

		for (size_t localVertexId = 0; localVertexId < 3; localVertexId++)
			positionIndex[localVertexId] = faces[(triangleId * 3 + localVertexId) * faceStride];

		if (vertexFormat == VertexFormat::POSITION_TEXCOORD || vertexFormat == VertexFormat::POSITION_NORMAL_TEXCOORD)
			for (uint32_t localVertexId = 0; localVertexId < 3; localVertexId++)
				texCoordIndex[localVertexId] = faces[(triangleId * 3 + localVertexId) * faceStride + 1];

		auto& position0 = positions[positionIndex[0]];
		auto& position1 = positions[positionIndex[1]];
		auto& position2 = positions[positionIndex[2]];

		float3 normal = CalculateNormal(position0, position1, position2);

		for (uint32_t localVertexId = 0; localVertexId < 3; localVertexId++)
		{
			newFaces.push_back(positionIndex[localVertexId]);
			if (vertexFormat == VertexFormat::POSITION_TEXCOORD || vertexFormat == VertexFormat::POSITION_NORMAL_TEXCOORD)
				newFaces.push_back(texCoordIndex[localVertexId]);
			newFaces.push_back(newNormals.size());
		}

		newNormals.push_back(normal);
	}

	faces = newFaces;
	normals = newNormals;
}

void Graphics::CalculateTangents(float3 normal, float3& tangent, float3& binormal)
{
	float3 v0(0.0f, 0.0f, 1.0f);
	float3 v1(0.0f, 1.0f, 0.0f);

	floatN t0 = XMVector3Cross(XMLoadFloat3(&normal), XMLoadFloat3(&v0));
	floatN t1 = XMVector3Cross(XMLoadFloat3(&normal), XMLoadFloat3(&v1));

	if (XMVector3Length(t0).m128_f32[0] > XMVector3Length(t1).m128_f32[0])
		XMStoreFloat3(&tangent, XMVector3Normalize(t0));
	else
		XMStoreFloat3(&tangent, XMVector3Normalize(t1));

	floatN rawBinormal = XMVector3Cross(XMLoadFloat3(&tangent), XMLoadFloat3(&normal));
	XMStoreFloat3(&binormal, XMVector3Normalize(rawBinormal));
}

void Graphics::CalculateTangents(const std::vector<float3>& normals, std::vector<float3>& tangents, std::vector<float3>& binormals)
{
	for (auto& normal : normals)
	{
		float3 tangent{};
		float3 binormal{};

		CalculateTangents(normal, tangent, binormal);

		tangents.push_back(tangent);
		binormals.push_back(binormal);
	}
}

void Graphics::SmoothNormals(VertexFormat vertexFormat, const std::vector<float3>& positions, std::vector<uint32_t>& faces, std::vector<float3>& normals)
{
	size_t faceStride = (vertexFormat == VertexFormat::POSITION_NORMAL_TEXCOORD) ? 3 : 2;

	std::vector<float3> newNormals;

	std::set<uint32_t> processedFaceIds;

	for (uint32_t faceId = 0; faceId < faces.size(); faceId += faceStride)
	{
		if (processedFaceIds.find(faceId) != processedFaceIds.end())
			continue;

		std::vector<uint32_t> samePositionFaceIndices = GetFaceIndicesForSamePosition(vertexFormat, positions, faces, faceId, positions[faces[faceId]]);

		floatN averageNormal{};

		for (auto& samePositionFaceIndex : samePositionFaceIndices)
		{
			auto normalSample = XMLoadFloat3(&normals[faces[samePositionFaceIndex + faceStride - 1]]);
			averageNormal += normalSample;
		}

		averageNormal = XMVector3Normalize(averageNormal);

		newNormals.push_back(float3(averageNormal.m128_f32[0], averageNormal.m128_f32[1], averageNormal.m128_f32[2]));

		for (auto& samePositionFaceIndex : samePositionFaceIndices)
		{
			faces[samePositionFaceIndex + faceStride - 1] = newNormals.size() - 1;
			processedFaceIds.insert(samePositionFaceIndex);
		}
	}

	normals = newNormals;
}

const std::vector<uint32_t> Graphics::GetFaceIndicesForSamePosition(VertexFormat vertexFormat, const std::vector<float3>& positions, const std::vector<uint32_t>& faces, uint32_t startFaceIndex, float3 position)
{
	const float epsilon = 0.00001f;

	std::vector<uint32_t> positionIndices;

	uint32_t faceStride = (vertexFormat == VertexFormat::POSITION) ? 1 : (vertexFormat == VertexFormat::POSITION_NORMAL_TEXCOORD) ? 3 : 2;

	for (uint32_t faceId = startFaceIndex; faceId < faces.size(); faceId += faceStride)
	{
		if (std::abs(positions[faces[faceId]].x - position.x) > epsilon)
			continue;

		if (std::abs(positions[faces[faceId]].y - position.y) > epsilon)
			continue;

		if (std::abs(positions[faces[faceId]].z - position.z) > epsilon)
			continue;

		positionIndices.push_back(faceId);
	}

	return positionIndices;
}

int64_t Graphics::GetIndexForSameVertex(uint32_t vertex4ByteStride, const std::vector<float>& vertices, const std::vector<uint32_t>& indices,
	float3 position, float3 normal, float2 texCoord)
{
	const float epsilon = 0.00001f;

	for (auto& vertexIndex : indices)
	{
		size_t stridedVertexIndex = static_cast<size_t>(vertexIndex) * vertex4ByteStride;

		if (std::abs(vertices[stridedVertexIndex] - position.x) > epsilon)
			continue;

		if (std::abs(vertices[stridedVertexIndex + 1] - position.y) > epsilon)
			continue;

		if (std::abs(vertices[stridedVertexIndex + 2] - position.z) > epsilon)
			continue;

		if (vertex4ByteStride == 5)
		{
			if (std::abs(vertices[stridedVertexIndex + 3] - texCoord.x) > epsilon)
				continue;

			if (std::abs(vertices[stridedVertexIndex + 4] - texCoord.y) > epsilon)
				continue;
		}
		else if (vertex4ByteStride > 5)
		{
			if (std::abs(vertices[stridedVertexIndex + 3] - normal.x) > epsilon)
				continue;

			if (std::abs(vertices[stridedVertexIndex + 4] - normal.y) > epsilon)
				continue;

			if (std::abs(vertices[stridedVertexIndex + 5] - normal.z) > epsilon)
				continue;

			if (vertex4ByteStride == 8)
			{
				if (std::abs(vertices[stridedVertexIndex + 6] - texCoord.x) > epsilon)
					continue;

				if (std::abs(vertices[stridedVertexIndex + 7] - texCoord.y) > epsilon)
					continue;
			}
			else if (vertex4ByteStride == 14)
			{
				if (std::abs(vertices[stridedVertexIndex + 12] - texCoord.x) > epsilon)
					continue;

				if (std::abs(vertices[stridedVertexIndex + 13] - texCoord.y) > epsilon)
					continue;
			}
		}

		return vertexIndex;
	}

	return -1;
}

uint32_t Graphics::GetVertex4ByteStride(VertexFormat vertexFormat)
{
	return GetVertexStride(vertexFormat) / 4;
}
