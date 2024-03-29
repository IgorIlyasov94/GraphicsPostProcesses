#include "ResourceManager.h"

Graphics::ResourceManager& Graphics::ResourceManager::GetInstance()
{
	static ResourceManager thisInstance;

	return thisInstance;
}

void Graphics::ResourceManager::Initialize(ID3D12Device* _device)
{
	device = _device;
	
	CreateCommandQueue(device, &commandQueue);

	ThrowIfFailed(device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)), "ResourceManager::Initialize: Fence creating error!");
	fenceValue++;

	fenceEvent = CreateEvent(nullptr, false, false, nullptr);
	if (fenceEvent == nullptr)
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()), "ResourceManager::Initialize: Fence Event creating error!");

	ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)),
		"ResourceManager::Initialize: Command Allocator creating error!");

	ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)),
		"ResourceManager::Initialize: Command List creating error!");
}

Graphics::VertexBufferId Graphics::ResourceManager::CreateVertexBuffer(const void* data, size_t dataSize, size_t vertexStride)
{
	BufferAllocation vertexBufferAllocation{};
	bufferAllocator.Allocate(device, dataSize, 64 * _KB, D3D12_HEAP_TYPE_DEFAULT, vertexBufferAllocation);

	BufferAllocation uploadBufferAllocation{};
	bufferAllocator.AllocateTemporary(device, dataSize, D3D12_HEAP_TYPE_UPLOAD, uploadBufferAllocation);

	if (vertexBufferAllocation.bufferResource == nullptr)
		throw std::exception("ResourceManager::CreateVertexBuffer: Vertex Buffer Resource is null!");

	if (uploadBufferAllocation.bufferResource == nullptr)
		throw std::exception("ResourceManager::CreateVertexBuffer: Upload Buffer Resource is null!");

	std::copy(reinterpret_cast<const uint8_t*>(data), reinterpret_cast<const uint8_t*>(data) + dataSize, uploadBufferAllocation.cpuAddress);

	commandList->CopyBufferRegion(vertexBufferAllocation.bufferResource, vertexBufferAllocation.gpuPageOffset, uploadBufferAllocation.bufferResource,
		0, uploadBufferAllocation.bufferResource->GetDesc().Width);

	SetResourceBarrier(commandList.Get(), vertexBufferAllocation.bufferResource, D3D12_RESOURCE_BARRIER_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	ExecuteGPUCommands();

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView.BufferLocation = vertexBufferAllocation.gpuAddress;
	vertexBufferView.SizeInBytes = static_cast<uint32_t>(dataSize);
	vertexBufferView.StrideInBytes = static_cast<uint32_t>(vertexStride);

	VertexBuffer vertexBuffer{};
	vertexBuffer.vertexBufferAllocation = vertexBufferAllocation;
	vertexBuffer.vertexBufferView = vertexBufferView;
	vertexBuffer.currentResourceState = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;

	vertexBufferPool.push_back(vertexBuffer);

	return VertexBufferId(vertexBufferPool.size() - 1);
}

Graphics::VertexBufferId Graphics::ResourceManager::CreateDynamicVertexBuffer(const void* data, size_t dataSize, size_t vertexStride)
{
	BufferAllocation vertexBufferAllocation{};
	bufferAllocator.Allocate(device, dataSize, 64 * _KB, D3D12_HEAP_TYPE_UPLOAD, vertexBufferAllocation);

	if (vertexBufferAllocation.bufferResource == nullptr)
		throw std::exception("ResourceManager::CreateDynamicVertexBuffer: Vertex Buffer Resource is null!");

	std::copy(reinterpret_cast<const uint8_t*>(data), reinterpret_cast<const uint8_t*>(data) + dataSize, vertexBufferAllocation.cpuAddress);

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView.BufferLocation = vertexBufferAllocation.gpuAddress;
	vertexBufferView.SizeInBytes = static_cast<uint32_t>(dataSize);
	vertexBufferView.StrideInBytes = static_cast<uint32_t>(vertexStride);

	VertexBuffer vertexBuffer{};
	vertexBuffer.vertexBufferAllocation = vertexBufferAllocation;
	vertexBuffer.vertexBufferView = vertexBufferView;
	vertexBuffer.currentResourceState = D3D12_RESOURCE_STATE_GENERIC_READ;

	vertexBufferPool.push_back(vertexBuffer);

	return VertexBufferId(vertexBufferPool.size() - 1);
}

Graphics::IndexBufferId Graphics::ResourceManager::CreateIndexBuffer(const void* data, size_t dataSize, size_t indexStride)
{
	BufferAllocation indexBufferAllocation{};
	bufferAllocator.Allocate(device, dataSize, 64 * _KB, D3D12_HEAP_TYPE_DEFAULT, indexBufferAllocation);
	
	BufferAllocation uploadBufferAllocation{};
	bufferAllocator.AllocateTemporary(device, dataSize, D3D12_HEAP_TYPE_UPLOAD, uploadBufferAllocation);

	if (indexBufferAllocation.bufferResource == nullptr)
		throw std::exception("ResourceManager::CreateIndexBuffer: Index Buffer Resource is null!");

	if (uploadBufferAllocation.bufferResource == nullptr)
		throw std::exception("ResourceManager::CreateIndexBuffer: Upload Buffer Resource is null!");

	std::copy(reinterpret_cast<const uint8_t*>(data), reinterpret_cast<const uint8_t*>(data) + dataSize, uploadBufferAllocation.cpuAddress);

	D3D12_INDEX_BUFFER_VIEW indexBufferView{};
	indexBufferView.BufferLocation = indexBufferAllocation.gpuAddress;
	indexBufferView.SizeInBytes = static_cast<uint32_t>(dataSize);
	indexBufferView.Format = (indexStride == 4) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;

	commandList->CopyBufferRegion(indexBufferAllocation.bufferResource, indexBufferAllocation.gpuPageOffset, uploadBufferAllocation.bufferResource,
		0, uploadBufferAllocation.bufferResource->GetDesc().Width);

	SetResourceBarrier(commandList.Get(), indexBufferAllocation.bufferResource, D3D12_RESOURCE_BARRIER_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_INDEX_BUFFER);

	ExecuteGPUCommands();

	IndexBuffer indexBuffer{};
	indexBuffer.indicesCount = static_cast<uint32_t>(dataSize / indexStride);
	indexBuffer.indexBufferView = indexBufferView;
	indexBuffer.indexBufferAllocation = indexBufferAllocation;
	indexBuffer.currentResourceState = D3D12_RESOURCE_STATE_INDEX_BUFFER;
	
	indexBufferPool.push_back(indexBuffer);

	return IndexBufferId(indexBufferPool.size() - 1);
}

Graphics::ConstantBufferId Graphics::ResourceManager::CreateConstantBuffer(const void* data, size_t dataSize)
{
	BufferAllocation constantBufferAllocation{};
	bufferAllocator.Allocate(device, dataSize, 64 * _KB, D3D12_HEAP_TYPE_UPLOAD, constantBufferAllocation);

	if (constantBufferAllocation.bufferResource == nullptr)
		throw std::exception("ResourceManager::CreateConstantBuffer: Buffer Resource is null!");

	DescriptorAllocation constantBufferDescriptorAllocation{};
	descriptorAllocator.Allocate(device, false, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, constantBufferDescriptorAllocation);

	D3D12_CONSTANT_BUFFER_VIEW_DESC constantBufferViewDesc{};
	constantBufferViewDesc.BufferLocation = constantBufferAllocation.gpuAddress;
	constantBufferViewDesc.SizeInBytes = static_cast<uint32_t>(AlignSize(dataSize, static_cast<size_t>(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT)));

	device->CreateConstantBufferView(&constantBufferViewDesc, constantBufferDescriptorAllocation.descriptorBase);

	std::copy(reinterpret_cast<const uint8_t*>(data), reinterpret_cast<const uint8_t*>(data) + dataSize, constantBufferAllocation.cpuAddress);

	ConstantBuffer constantBuffer{};
	constantBuffer.uploadBufferAllocation = constantBufferAllocation;
	constantBuffer.bufferDescriptorAllocation = constantBufferDescriptorAllocation;
	constantBuffer.constantBufferViewDesc = constantBufferViewDesc;
	constantBuffer.currentResourceState = D3D12_RESOURCE_STATE_GENERIC_READ;

	constantBufferPool.push_back(constantBuffer);

	return ConstantBufferId(constantBufferPool.size() - 1);
}

Graphics::TextureId Graphics::ResourceManager::CreateTexture(const std::filesystem::path& fileName)
{
	std::vector<uint8_t> textureData;
	TextureInfo textureInfo;

	if (fileName.extension() == ".dds")
		DDSLoader::Load(fileName, textureData, textureInfo);
	else
		throw std::exception("ResourceManager::CreateTexture: Unsupported file format!");

	return CreateTexture(textureData, textureInfo, D3D12_RESOURCE_FLAG_NONE);
}

Graphics::TextureId Graphics::ResourceManager::CreateTexture(const std::vector<uint8_t>& data, const TextureInfo& textureInfo, D3D12_RESOURCE_FLAGS resourceFlags)
{
	TextureAllocation textureAllocation{};
	textureAllocator.Allocate(device, resourceFlags, nullptr, textureInfo, textureAllocation);

	TextureAllocation uploadTextureAllocation{};
	textureAllocator.AllocateTemporaryUpload(device, resourceFlags, textureInfo, uploadTextureAllocation);

	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	shaderResourceViewDesc.Format = textureInfo.format;
	shaderResourceViewDesc.ViewDimension = textureInfo.srvDimension;
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	if (textureInfo.srvDimension == D3D12_SRV_DIMENSION_TEXTURE1D)
	{
		shaderResourceViewDesc.Texture1D.MipLevels = textureInfo.mipLevels;
	}
	else if (textureInfo.srvDimension == D3D12_SRV_DIMENSION_TEXTURE1DARRAY)
	{
		shaderResourceViewDesc.Texture1DArray.MipLevels = textureInfo.mipLevels;
		shaderResourceViewDesc.Texture1DArray.ArraySize = textureInfo.depth;
	}
	else if (textureInfo.srvDimension == D3D12_SRV_DIMENSION_TEXTURE2D)
	{
		shaderResourceViewDesc.Texture2D.MipLevels = textureInfo.mipLevels;
	}
	else if (textureInfo.srvDimension == D3D12_SRV_DIMENSION_TEXTURE2DARRAY)
	{
		shaderResourceViewDesc.Texture2DArray.MipLevels = textureInfo.mipLevels;
		shaderResourceViewDesc.Texture2DArray.ArraySize = textureInfo.depth;
	}
	else if (textureInfo.srvDimension == D3D12_SRV_DIMENSION_TEXTURECUBE)
	{
		shaderResourceViewDesc.TextureCube.MipLevels = textureInfo.mipLevels;
	}
	else if (textureInfo.srvDimension == D3D12_SRV_DIMENSION_TEXTURECUBEARRAY)
	{
		shaderResourceViewDesc.TextureCubeArray.MipLevels = textureInfo.mipLevels;
		shaderResourceViewDesc.TextureCubeArray.NumCubes = textureInfo.depth;
	}
	else if (textureInfo.srvDimension == D3D12_SRV_DIMENSION_TEXTURE3D)
	{
		shaderResourceViewDesc.Texture3D.MipLevels = textureInfo.mipLevels;
	}

	if (textureAllocation.textureResource == nullptr)
		throw std::exception("ResourceManager::CreateTexture: Texture Resource is null!");

	if (uploadTextureAllocation.textureResource == nullptr)
		throw std::exception("ResourceManager::CreateTexture: Upload Texture Resource is null!");

	SetResourceBarrier(commandList.Get(), textureAllocation.textureResource, D3D12_RESOURCE_BARRIER_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);

	UploadTexture(uploadTextureAllocation.textureResource, textureAllocation.textureResource, textureInfo, data, uploadTextureAllocation.cpuAddress);

	SetResourceBarrier(commandList.Get(), textureAllocation.textureResource, D3D12_RESOURCE_BARRIER_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);

	ExecuteGPUCommands();

	DescriptorAllocation shaderResourceDescriptorAllocation{};
	descriptorAllocator.Allocate(device, false, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shaderResourceDescriptorAllocation);
	device->CreateShaderResourceView(textureAllocation.textureResource, &shaderResourceViewDesc, shaderResourceDescriptorAllocation.descriptorBase);

	Texture texture{};
	texture.shaderResourceViewDesc = shaderResourceViewDesc;
	texture.info = textureInfo;
	texture.textureAllocation = textureAllocation;
	texture.shaderResourceDescriptorAllocation = shaderResourceDescriptorAllocation;
	texture.currentResourceState = D3D12_RESOURCE_STATE_COMMON;

	texturePool.push_back(texture);

	return TextureId(texturePool.size() - 1);
}

Graphics::BufferId Graphics::ResourceManager::CreateBuffer(const void* data, size_t dataSize, size_t bufferStride, size_t numElements, DXGI_FORMAT format)
{
	BufferAllocation bufferAllocation{};
	bufferAllocator.AllocateCustomBuffer(device, dataSize, 64 * _KB, bufferAllocation);

	BufferAllocation uploadBufferAllocation{};
	bufferAllocator.AllocateTemporary(device, dataSize, D3D12_HEAP_TYPE_UPLOAD, uploadBufferAllocation);

	if (bufferAllocation.bufferResource == nullptr)
		throw std::exception("ResourceManager::CreateBuffer: Buffer Resource is null!");

	if (uploadBufferAllocation.bufferResource == nullptr)
		throw std::exception("ResourceManager::CreateBuffer: Upload Buffer Resource is null!");

	std::copy(reinterpret_cast<const uint8_t*>(data), reinterpret_cast<const uint8_t*>(data) + dataSize, uploadBufferAllocation.cpuAddress);

	commandList->CopyBufferRegion(bufferAllocation.bufferResource, bufferAllocation.gpuPageOffset, uploadBufferAllocation.bufferResource,
		0, uploadBufferAllocation.bufferResource->GetDesc().Width);

	ExecuteGPUCommands();

	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	shaderResourceViewDesc.Format = (bufferStride == 0) ? format : (bufferStride == 1) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_UNKNOWN;
	shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceViewDesc.Buffer.Flags = (bufferStride == 1) ? D3D12_BUFFER_SRV_FLAG_RAW : D3D12_BUFFER_SRV_FLAG_NONE;
	shaderResourceViewDesc.Buffer.NumElements = static_cast<uint32_t>(numElements);
	shaderResourceViewDesc.Buffer.StructureByteStride = static_cast<uint32_t>(bufferStride);

	DescriptorAllocation shaderResourceDescriptorAllocation{};
	descriptorAllocator.Allocate(device, false, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shaderResourceDescriptorAllocation);
	device->CreateShaderResourceView(bufferAllocation.bufferResource, &shaderResourceViewDesc, shaderResourceDescriptorAllocation.descriptorBase);

	Buffer buffer{};
	buffer.bufferAllocation = bufferAllocation;
	buffer.shaderResourceViewDesc = shaderResourceViewDesc;
	buffer.shaderResourceDescriptorAllocation = shaderResourceDescriptorAllocation;
	buffer.currentResourceState = D3D12_RESOURCE_STATE_COPY_DEST;

	bufferPool.push_back(buffer);

	return BufferId(bufferPool.size() - 1);
}

Graphics::SamplerId Graphics::ResourceManager::CreateSampler(const D3D12_SAMPLER_DESC& samplerDesc)
{
	DescriptorAllocation samplerDescriptorAllocation{};
	descriptorAllocator.Allocate(device, false, 1, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, samplerDescriptorAllocation);

	device->CreateSampler(&samplerDesc, samplerDescriptorAllocation.descriptorBase);

	Sampler sampler{};
	sampler.samplerDesc = samplerDesc;
	sampler.samplerDescriptorAllocation = samplerDescriptorAllocation;

	samplerPool.push_back(sampler);

	return SamplerId(samplerPool.size() - 1);
}

Graphics::RenderTargetId Graphics::ResourceManager::CreateRenderTarget(uint64_t width, uint32_t height, DXGI_FORMAT format)
{
	TextureInfo textureInfo{};
	textureInfo.width = width;
	textureInfo.height = height;
	textureInfo.depth = 1;
	textureInfo.mipLevels = 1;
	textureInfo.format = format;
	textureInfo.dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureInfo.srvDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = format;

	TextureAllocation textureAllocation{};
	textureAllocator.Allocate(device, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, &clearValue, textureInfo, textureAllocation);
	
	if (textureAllocation.textureResource == nullptr)
		throw std::exception("ResourceManager::CreateRenderTarget: Texture Resource is null!");

	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	shaderResourceViewDesc.Format = format;
	shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	DescriptorAllocation shaderResourceDescriptorAllocation{};
	descriptorAllocator.Allocate(device, false, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shaderResourceDescriptorAllocation);
	device->CreateShaderResourceView(textureAllocation.textureResource, &shaderResourceViewDesc, shaderResourceDescriptorAllocation.descriptorBase);

	D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc{};
	renderTargetViewDesc.Format = format;
	renderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	
	DescriptorAllocation renderTargetDescriptorAllocation{};
	descriptorAllocator.Allocate(device, false, 1, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, renderTargetDescriptorAllocation);

	device->CreateRenderTargetView(textureAllocation.textureResource, &renderTargetViewDesc, renderTargetDescriptorAllocation.descriptorBase);

	RenderTarget renderTarget{};
	renderTarget.shaderResourceViewDesc = shaderResourceViewDesc;
	renderTarget.renderTargetViewDesc = renderTargetViewDesc;
	renderTarget.info = textureInfo;
	renderTarget.textureAllocation = textureAllocation;
	renderTarget.renderTargetDescriptorAllocation = renderTargetDescriptorAllocation;
	renderTarget.shaderResourceDescriptorAllocation = shaderResourceDescriptorAllocation;
	renderTarget.currentResourceState = D3D12_RESOURCE_STATE_RENDER_TARGET;

	renderTargetPool.push_back(renderTarget);

	return RenderTargetId(renderTargetPool.size() - 1);
}

Graphics::DepthStencilId Graphics::ResourceManager::CreateDepthStencil(uint64_t width, uint32_t height, uint32_t depthBit)
{
	TextureInfo textureInfo{};
	textureInfo.width = width;
	textureInfo.height = height;
	textureInfo.depth = 1;
	textureInfo.mipLevels = 1;
	textureInfo.format = (depthBit == 32) ? DXGI_FORMAT_R32_TYPELESS : DXGI_FORMAT_R24G8_TYPELESS;
	textureInfo.dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureInfo.srvDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = (depthBit == 32) ? DXGI_FORMAT_D32_FLOAT : DXGI_FORMAT_D24_UNORM_S8_UINT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	TextureAllocation textureAllocation{};
	textureAllocator.Allocate(device, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, &clearValue, textureInfo, textureAllocation);

	if (textureAllocation.textureResource == nullptr)
		throw std::exception("ResourceManager::CreateDepthStencil: Texture Resource is null!");

	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	shaderResourceViewDesc.Format = (depthBit == 32) ? DXGI_FORMAT_R32_FLOAT : DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	DescriptorAllocation shaderResourceDescriptorAllocation{};
	descriptorAllocator.Allocate(device, false, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shaderResourceDescriptorAllocation);
	device->CreateShaderResourceView(textureAllocation.textureResource, &shaderResourceViewDesc, shaderResourceDescriptorAllocation.descriptorBase);

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = (depthBit == 32) ? DXGI_FORMAT_D32_FLOAT : DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	DescriptorAllocation depthStencilDescriptorAllocation{};
	descriptorAllocator.Allocate(device, false, 1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, depthStencilDescriptorAllocation);
	device->CreateDepthStencilView(textureAllocation.textureResource, &depthStencilViewDesc, depthStencilDescriptorAllocation.descriptorBase);

	DepthStencil depthStencil{};
	depthStencil.shaderResourceViewDesc = shaderResourceViewDesc;
	depthStencil.depthStencilViewDesc = depthStencilViewDesc;
	depthStencil.info = textureInfo;
	depthStencil.textureAllocation = textureAllocation;
	depthStencil.depthStencilDescriptorAllocation = depthStencilDescriptorAllocation;
	depthStencil.shaderResourceDescriptorAllocation = shaderResourceDescriptorAllocation;
	depthStencil.currentResourceState = D3D12_RESOURCE_STATE_DEPTH_WRITE;

	depthStencilPool.push_back(depthStencil);

	return DepthStencilId(depthStencilPool.size() - 1);
}

Graphics::RWTextureId Graphics::ResourceManager::CreateRWTexture(const TextureInfo& textureInfo)
{
	TextureAllocation textureAllocation{};
	textureAllocator.Allocate(device, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, nullptr, textureInfo, textureAllocation);

	if (textureAllocation.textureResource == nullptr)
		throw std::exception("ResourceManager::CreateRWTexture: Texture Resource is null!");

	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	shaderResourceViewDesc.Format = textureInfo.format;
	shaderResourceViewDesc.ViewDimension = textureInfo.srvDimension;
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	D3D12_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc{};
	unorderedAccessViewDesc.Format = textureInfo.format;

	if (shaderResourceViewDesc.ViewDimension == D3D12_SRV_DIMENSION_TEXTURE1D)
	{
		shaderResourceViewDesc.Texture1D.MipLevels = 1;
		unorderedAccessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
	}
	else if (shaderResourceViewDesc.ViewDimension == D3D12_SRV_DIMENSION_TEXTURE2D)
	{
		shaderResourceViewDesc.Texture2D.MipLevels = 1;
		unorderedAccessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	}
	else if (shaderResourceViewDesc.ViewDimension == D3D12_SRV_DIMENSION_TEXTURE3D)
	{
		shaderResourceViewDesc.Texture3D.MipLevels = 1;
		unorderedAccessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
		unorderedAccessViewDesc.Texture3D.WSize = textureInfo.depth;
	}
	else if (shaderResourceViewDesc.ViewDimension == D3D12_SRV_DIMENSION_TEXTURE1DARRAY)
	{
		shaderResourceViewDesc.Texture1DArray.MipLevels = 1;
		shaderResourceViewDesc.Texture1DArray.ArraySize = textureInfo.depth;
		unorderedAccessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
		unorderedAccessViewDesc.Texture1DArray.ArraySize = textureInfo.depth;
	}
	else if (shaderResourceViewDesc.ViewDimension == D3D12_SRV_DIMENSION_TEXTURE2DARRAY)
	{
		shaderResourceViewDesc.Texture2DArray.MipLevels = 1;
		shaderResourceViewDesc.Texture2DArray.ArraySize = textureInfo.depth;
		unorderedAccessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		unorderedAccessViewDesc.Texture2DArray.ArraySize = textureInfo.depth;
	}
	
	DescriptorAllocation shaderResourceDescriptorAllocation{};
	descriptorAllocator.Allocate(device, false, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shaderResourceDescriptorAllocation);
	device->CreateShaderResourceView(textureAllocation.textureResource, &shaderResourceViewDesc, shaderResourceDescriptorAllocation.descriptorBase);

	DescriptorAllocation unorderedAccessDescriptorAllocation{};
	descriptorAllocator.Allocate(device, false, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, unorderedAccessDescriptorAllocation);
	device->CreateUnorderedAccessView(textureAllocation.textureResource, nullptr, &unorderedAccessViewDesc, unorderedAccessDescriptorAllocation.descriptorBase);
	
	DescriptorAllocation shaderNonVisibleDescriptorAllocation{};
	descriptorAllocator.Allocate(device, true, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shaderNonVisibleDescriptorAllocation);
	device->CreateUnorderedAccessView(textureAllocation.textureResource, nullptr, &unorderedAccessViewDesc, shaderNonVisibleDescriptorAllocation.descriptorBase);

	SetResourceBarrier(commandList.Get(), textureAllocation.textureResource, D3D12_RESOURCE_BARRIER_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	ExecuteGPUCommands();

	RWTexture rwTexture{};
	rwTexture.shaderResourceViewDesc = shaderResourceViewDesc;
	rwTexture.unorderedAccessViewDesc = unorderedAccessViewDesc;
	rwTexture.info = textureInfo;
	rwTexture.textureAllocation = textureAllocation;
	rwTexture.unorderedAccessDescriptorAllocation = unorderedAccessDescriptorAllocation;
	rwTexture.shaderResourceDescriptorAllocation = shaderResourceDescriptorAllocation;
	rwTexture.shaderNonVisibleDescriptorAllocation = shaderNonVisibleDescriptorAllocation;
	rwTexture.currentResourceState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

	rwTexturePool.push_back(rwTexture);

	return RWTextureId(rwTexturePool.size() - 1);
}

Graphics::RWBufferId Graphics::ResourceManager::CreateRWBuffer(const void* initialData, size_t dataSize, size_t bufferStride, size_t numElements, DXGI_FORMAT format,
	bool addCounter)
{
	BufferAllocation bufferAllocation{};
	bufferAllocator.AllocateUnorderedAccess(device, dataSize, 64 * _KB, bufferAllocation);

	BufferAllocation uploadBufferAllocation{};
	bufferAllocator.AllocateTemporary(device, dataSize, D3D12_HEAP_TYPE_UPLOAD, uploadBufferAllocation);

	if (bufferAllocation.bufferResource == nullptr)
		throw std::exception("ResourceManager::CreateRWBuffer: RWBuffer Resource is null!");

	if (uploadBufferAllocation.bufferResource == nullptr)
		throw std::exception("ResourceManager::CreateRWBuffer: Upload Buffer Resource is null!");

	std::copy(reinterpret_cast<const uint8_t*>(initialData), reinterpret_cast<const uint8_t*>(initialData) + dataSize, uploadBufferAllocation.cpuAddress);

	SetResourceBarrier(commandList.Get(), bufferAllocation.bufferResource, D3D12_RESOURCE_BARRIER_FLAG_NONE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_COPY_DEST);

	commandList->CopyBufferRegion(bufferAllocation.bufferResource, bufferAllocation.gpuPageOffset, uploadBufferAllocation.bufferResource,
		0, uploadBufferAllocation.bufferResource->GetDesc().Width);

	SetResourceBarrier(commandList.Get(), bufferAllocation.bufferResource, D3D12_RESOURCE_BARRIER_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	ExecuteGPUCommands();

	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	shaderResourceViewDesc.Format = (bufferStride == 0) ? format : (bufferStride == 1) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_UNKNOWN;
	shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceViewDesc.Buffer.Flags = (bufferStride == 1) ? D3D12_BUFFER_SRV_FLAG_RAW : D3D12_BUFFER_SRV_FLAG_NONE;
	shaderResourceViewDesc.Buffer.NumElements = static_cast<uint32_t>(numElements);
	shaderResourceViewDesc.Buffer.StructureByteStride = (bufferStride > 1) ? static_cast<uint32_t>(bufferStride) : 0u;

	DescriptorAllocation shaderResourceDescriptorAllocation{};
	descriptorAllocator.Allocate(device, false, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shaderResourceDescriptorAllocation);
	device->CreateShaderResourceView(bufferAllocation.bufferResource, &shaderResourceViewDesc, shaderResourceDescriptorAllocation.descriptorBase);

	D3D12_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc{};
	unorderedAccessViewDesc.Format = (bufferStride == 0) ? format : (bufferStride == 1) ? DXGI_FORMAT_R32_TYPELESS : DXGI_FORMAT_UNKNOWN;
	unorderedAccessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	unorderedAccessViewDesc.Buffer.Flags = (bufferStride == 1) ? D3D12_BUFFER_UAV_FLAG_RAW : D3D12_BUFFER_UAV_FLAG_NONE;
	unorderedAccessViewDesc.Buffer.NumElements = static_cast<uint32_t>(numElements);
	unorderedAccessViewDesc.Buffer.StructureByteStride = (bufferStride > 1) ? static_cast<uint32_t>(bufferStride) : 0u;
	
	DescriptorAllocation unorderedAccessDescriptorAllocation{};
	descriptorAllocator.Allocate(device, false, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, unorderedAccessDescriptorAllocation);

	DescriptorAllocation shaderNonVisibleDescriptorAllocation{};
	
	if (bufferStride <= 1)
	{
		descriptorAllocator.Allocate(device, true, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shaderNonVisibleDescriptorAllocation);

		device->CreateUnorderedAccessView(bufferAllocation.bufferResource, nullptr, &unorderedAccessViewDesc, unorderedAccessDescriptorAllocation.descriptorBase);
		device->CreateUnorderedAccessView(bufferAllocation.bufferResource, nullptr, &unorderedAccessViewDesc, shaderNonVisibleDescriptorAllocation.descriptorBase);
	}
	else
	{
		BufferAllocation counterBufferAllocation{};

		if (addCounter)
		{
			bufferAllocator.AllocateUnorderedAccess(device, 4, 64 * _KB, counterBufferAllocation);

			if (counterBufferAllocation.bufferResource == nullptr)
				throw std::exception("ResourceManager::CreateRWBuffer: Counter Resource is null!");
		}

		device->CreateUnorderedAccessView(bufferAllocation.bufferResource, counterBufferAllocation.bufferResource, &unorderedAccessViewDesc,
			unorderedAccessDescriptorAllocation.descriptorBase);
	}

	RWBuffer rwBuffer{};
	rwBuffer.bufferAllocation = bufferAllocation;
	rwBuffer.shaderResourceViewDesc = shaderResourceViewDesc;
	rwBuffer.unorderedAccessViewDesc = unorderedAccessViewDesc;
	rwBuffer.unorderedAccessDescriptorAllocation = unorderedAccessDescriptorAllocation;
	rwBuffer.shaderResourceDescriptorAllocation = shaderResourceDescriptorAllocation;
	rwBuffer.shaderNonVisibleDescriptorAllocation = shaderNonVisibleDescriptorAllocation;
	rwBuffer.currentResourceState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	
	rwBufferPool.push_back(rwBuffer);

	return RWBufferId(rwBufferPool.size() - 1);
}

void Graphics::ResourceManager::CreateSwapChainBuffers(IDXGISwapChain4* swapChain, uint32_t buffersCount)
{
	for (uint32_t bufferId = 0; bufferId < buffersCount; bufferId++)
	{
		DescriptorAllocation renderTargetDescriptorAllocation{};

		descriptorAllocator.Allocate(device, false, buffersCount, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, renderTargetDescriptorAllocation);
		swapChainDescriptorBases.push_back(renderTargetDescriptorAllocation.descriptorBase);

		swapChainBuffers.push_back(nullptr);
		swapChain->GetBuffer(bufferId, IID_PPV_ARGS(&swapChainBuffers.back()));

		device->CreateRenderTargetView(swapChainBuffers.back().Get(), nullptr, swapChainDescriptorBases.back());

		swapChainStates.push_back(D3D12_RESOURCE_STATE_PRESENT);
	}
}

const Graphics::VertexBuffer& Graphics::ResourceManager::GetVertexBuffer(const VertexBufferId& resourceId) const
{
	return vertexBufferPool[resourceId.value];
}

const Graphics::IndexBuffer& Graphics::ResourceManager::GetIndexBuffer(const IndexBufferId& resourceId) const
{
	return indexBufferPool[resourceId.value];
}

const Graphics::ConstantBuffer& Graphics::ResourceManager::GetConstantBuffer(const ConstantBufferId& resourceId) const
{
	return constantBufferPool[resourceId.value];
}

const Graphics::Texture& Graphics::ResourceManager::GetTexture(const TextureId& resourceId) const
{
	return texturePool[resourceId.value];
}

const Graphics::Buffer& Graphics::ResourceManager::GetBuffer(const BufferId& resourceId) const
{
	return bufferPool[resourceId.value];
}

const Graphics::Sampler& Graphics::ResourceManager::GetSampler(const SamplerId& resourceId) const
{
	return samplerPool[resourceId.value];
}

const Graphics::RenderTarget& Graphics::ResourceManager::GetRenderTarget(const RenderTargetId& resourceId) const
{
	return renderTargetPool[resourceId.value];
}

const Graphics::DepthStencil& Graphics::ResourceManager::GetDepthStencil(const DepthStencilId& resourceId) const
{
	return depthStencilPool[resourceId.value];
}

const Graphics::RWTexture& Graphics::ResourceManager::GetRWTexture(const RWTextureId& resourceId) const
{
	return rwTexturePool[resourceId.value];
}

const Graphics::RWBuffer& Graphics::ResourceManager::GetRWBuffer(const RWBufferId& resourceId) const
{
	return rwBufferPool[resourceId.value];
}

const D3D12_CPU_DESCRIPTOR_HANDLE& Graphics::ResourceManager::GetSwapChainDescriptorBase(uint32_t bufferId) const
{
	return swapChainDescriptorBases[bufferId];
}

ID3D12Resource* Graphics::ResourceManager::GetSwapChainBuffer(uint32_t bufferId) const
{
	return swapChainBuffers[bufferId].Get();
}

void Graphics::ResourceManager::ResetSwapChainBuffers(IDXGISwapChain4* swapChain)
{
	for (auto& swapChainBuffer : swapChainBuffers)
		swapChainBuffer.Reset();

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChain->GetDesc1(&swapChainDesc);

	ThrowIfFailed(swapChain->ResizeBuffers(static_cast<uint32_t>(swapChainBuffers.size()), swapChainDesc.Width, swapChainDesc.Height, swapChainDesc.Format,
		swapChainDesc.Flags), "ResourceManager::ResetSwapChainBuffers: Back buffers resizing error!");

	for (uint32_t swapChainBufferId = 0; swapChainBufferId < swapChainBuffers.size(); swapChainBufferId++)
	{
		swapChain->GetBuffer(swapChainBufferId, IID_PPV_ARGS(&swapChainBuffers[swapChainBufferId]));
		device->CreateRenderTargetView(swapChainBuffers[swapChainBufferId].Get(), nullptr, swapChainDescriptorBases[swapChainBufferId]);
	}
}

D3D12_VERTEX_BUFFER_VIEW Graphics::ResourceManager::GetVertexBufferView(const VertexBufferId& resourceId) const
{
	return vertexBufferPool[resourceId.value].vertexBufferView;
}

D3D12_INDEX_BUFFER_VIEW Graphics::ResourceManager::GetIndexBufferView(const IndexBufferId& resourceId) const
{
	return indexBufferPool[resourceId.value].indexBufferView;
}

const D3D12_CPU_DESCRIPTOR_HANDLE& Graphics::ResourceManager::GetRenderTargetDescriptorBase(const RenderTargetId& resourceId) const
{
	return renderTargetPool[resourceId.value].renderTargetDescriptorAllocation.descriptorBase;
}

const D3D12_CPU_DESCRIPTOR_HANDLE& Graphics::ResourceManager::GetDepthStencilDescriptorBase(const DepthStencilId& resourceId) const
{
	return depthStencilPool[resourceId.value].depthStencilDescriptorAllocation.descriptorBase;
}

ID3D12DescriptorHeap* Graphics::ResourceManager::GetShaderResourceViewDescriptorHeap()
{
	if (texturePool.empty())
		return nullptr;

	return texturePool.front().shaderResourceDescriptorAllocation.descriptorHeap;
}

void Graphics::ResourceManager::GetTextureDataFromGPU(TextureId textureId, std::vector<float4>& rawTextureData)
{
	auto& resource = texturePool[textureId.value];

	GetTextureDataFromGPU(resource.textureAllocation.textureResource, resource.info, resource.currentResourceState, rawTextureData);
}

void Graphics::ResourceManager::GetTextureDataFromGPU(RenderTargetId textureId, std::vector<float4>& rawTextureData)
{
	auto& resource = renderTargetPool[textureId.value];

	GetTextureDataFromGPU(resource.textureAllocation.textureResource, resource.info, resource.currentResourceState, rawTextureData);
}

void Graphics::ResourceManager::GetTextureDataFromGPU(DepthStencilId textureId, std::vector<float4>& rawTextureData)
{
	auto& resource = depthStencilPool[textureId.value];

	GetTextureDataFromGPU(resource.textureAllocation.textureResource, resource.info, resource.currentResourceState, rawTextureData);
}

void Graphics::ResourceManager::GetTextureDataFromGPU(RWTextureId textureId, std::vector<float4>& rawTextureData)
{
	auto& resource = rwTexturePool[textureId.value];

	GetTextureDataFromGPU(resource.textureAllocation.textureResource, resource.info, resource.currentResourceState, rawTextureData);
}

void Graphics::ResourceManager::GetBufferDataFromGPU(VertexBufferId bufferId, std::vector<uint8_t>& rawBufferData)
{
	auto& resource = vertexBufferPool[bufferId.value];

	GetBufferDataFromGPU(resource.vertexBufferAllocation.bufferResource, resource.vertexBufferAllocation.nonAlignedSizeInBytes, resource.currentResourceState,
		rawBufferData);
}

void Graphics::ResourceManager::GetBufferDataFromGPU(IndexBufferId bufferId, std::vector<uint8_t>& rawBufferData)
{
	auto& resource = indexBufferPool[bufferId.value];

	GetBufferDataFromGPU(resource.indexBufferAllocation.bufferResource, resource.indexBufferAllocation.nonAlignedSizeInBytes, resource.currentResourceState,
		rawBufferData);
}

void Graphics::ResourceManager::GetBufferDataFromGPU(ConstantBufferId bufferId, std::vector<uint8_t>& rawBufferData)
{
	auto& resource = constantBufferPool[bufferId.value];

	GetBufferDataFromGPU(resource.uploadBufferAllocation.bufferResource, resource.uploadBufferAllocation.nonAlignedSizeInBytes, resource.currentResourceState,
		rawBufferData);
}

void Graphics::ResourceManager::GetBufferDataFromGPU(BufferId bufferId, std::vector<uint8_t>& rawBufferData)
{
	auto& resource = bufferPool[bufferId.value];

	GetBufferDataFromGPU(resource.bufferAllocation.bufferResource, resource.bufferAllocation.nonAlignedSizeInBytes, resource.currentResourceState, rawBufferData);
}

void Graphics::ResourceManager::GetBufferDataFromGPU(RWBufferId bufferId, std::vector<uint8_t>& rawBufferData)
{
	auto& resource = rwBufferPool[bufferId.value];

	GetBufferDataFromGPU(resource.bufferAllocation.bufferResource, resource.bufferAllocation.nonAlignedSizeInBytes, resource.currentResourceState, rawBufferData);
}

void Graphics::ResourceManager::UpdateDynamicVertexBuffer(const VertexBufferId& resourceId, const void* data, size_t dataSize)
{
	std::copy(reinterpret_cast<const uint8_t*>(data), reinterpret_cast<const uint8_t*>(data) + dataSize,
		vertexBufferPool[resourceId.value].vertexBufferAllocation.cpuAddress);
}

void Graphics::ResourceManager::UpdateConstantBuffer(const ConstantBufferId& resourceId, const void* data, size_t dataSize)
{
	std::copy(reinterpret_cast<const uint8_t*>(data), reinterpret_cast<const uint8_t*>(data) + dataSize,
		constantBufferPool[resourceId.value].uploadBufferAllocation.cpuAddress);
}

void Graphics::ResourceManager::SetResourceBarrier(ID3D12GraphicsCommandList* _commandList, const VertexBufferId& resourceId, D3D12_RESOURCE_BARRIER_FLAGS resourceBarrierFlags,
	D3D12_RESOURCE_STATES resourceBarrierStateAfter)
{
	auto& resource = vertexBufferPool[resourceId.value];

	SetResourceBarrier(_commandList, resource.vertexBufferAllocation.bufferResource, resourceBarrierFlags, resource.currentResourceState, resourceBarrierStateAfter);

	if (resourceBarrierFlags != D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY)
		resource.currentResourceState = resourceBarrierStateAfter;
}

void Graphics::ResourceManager::SetResourceBarrier(ID3D12GraphicsCommandList* _commandList, const IndexBufferId& resourceId, D3D12_RESOURCE_BARRIER_FLAGS resourceBarrierFlags,
	D3D12_RESOURCE_STATES resourceBarrierStateAfter)
{
	auto& resource = indexBufferPool[resourceId.value];

	SetResourceBarrier(_commandList, resource.indexBufferAllocation.bufferResource, resourceBarrierFlags, resource.currentResourceState, resourceBarrierStateAfter);

	if (resourceBarrierFlags != D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY)
		resource.currentResourceState = resourceBarrierStateAfter;
}

void Graphics::ResourceManager::SetResourceBarrier(ID3D12GraphicsCommandList* _commandList, const TextureId& resourceId, D3D12_RESOURCE_BARRIER_FLAGS resourceBarrierFlags,
	D3D12_RESOURCE_STATES resourceBarrierStateAfter)
{
	auto& resource = texturePool[resourceId.value];

	SetResourceBarrier(_commandList, resource.textureAllocation.textureResource, resourceBarrierFlags, resource.currentResourceState, resourceBarrierStateAfter);

	if (resourceBarrierFlags != D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY)
		resource.currentResourceState = resourceBarrierStateAfter;
}

void Graphics::ResourceManager::SetResourceBarrier(ID3D12GraphicsCommandList* _commandList, const BufferId& resourceId, D3D12_RESOURCE_BARRIER_FLAGS resourceBarrierFlags,
	D3D12_RESOURCE_STATES resourceBarrierStateAfter)
{
	auto& resource = bufferPool[resourceId.value];

	SetResourceBarrier(_commandList, resource.bufferAllocation.bufferResource, resourceBarrierFlags, resource.currentResourceState, resourceBarrierStateAfter);

	if (resourceBarrierFlags != D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY)
		resource.currentResourceState = resourceBarrierStateAfter;
}

void Graphics::ResourceManager::SetResourceBarrier(ID3D12GraphicsCommandList* _commandList, const RenderTargetId& resourceId, D3D12_RESOURCE_BARRIER_FLAGS resourceBarrierFlags,
	D3D12_RESOURCE_STATES resourceBarrierStateAfter)
{
	auto& resource = renderTargetPool[resourceId.value];

	SetResourceBarrier(_commandList, resource.textureAllocation.textureResource, resourceBarrierFlags, resource.currentResourceState, resourceBarrierStateAfter);

	if (resourceBarrierFlags != D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY)
		resource.currentResourceState = resourceBarrierStateAfter;
}

void Graphics::ResourceManager::SetResourceBarrier(ID3D12GraphicsCommandList* _commandList, const DepthStencilId& resourceId, D3D12_RESOURCE_BARRIER_FLAGS resourceBarrierFlags,
	D3D12_RESOURCE_STATES resourceBarrierStateAfter)
{
	auto& resource = depthStencilPool[resourceId.value];

	SetResourceBarrier(_commandList, resource.textureAllocation.textureResource, resourceBarrierFlags, resource.currentResourceState, resourceBarrierStateAfter);

	if (resourceBarrierFlags != D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY)
		resource.currentResourceState = resourceBarrierStateAfter;
}

void Graphics::ResourceManager::SetResourceBarrier(ID3D12GraphicsCommandList* _commandList, const RWTextureId& resourceId, D3D12_RESOURCE_BARRIER_FLAGS resourceBarrierFlags,
	D3D12_RESOURCE_STATES resourceBarrierStateAfter)
{
	auto& resource = rwTexturePool[resourceId.value];

	SetResourceBarrier(_commandList, resource.textureAllocation.textureResource, resourceBarrierFlags, resource.currentResourceState, resourceBarrierStateAfter);

	if (resourceBarrierFlags != D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY)
		resource.currentResourceState = resourceBarrierStateAfter;
}

void Graphics::ResourceManager::SetResourceBarrier(ID3D12GraphicsCommandList* _commandList, const RWBufferId& resourceId, D3D12_RESOURCE_BARRIER_FLAGS resourceBarrierFlags,
	D3D12_RESOURCE_STATES resourceBarrierStateAfter)
{
	auto& resource = rwBufferPool[resourceId.value];

	SetResourceBarrier(_commandList, resource.bufferAllocation.bufferResource, resourceBarrierFlags, resource.currentResourceState, resourceBarrierStateAfter);

	if (resourceBarrierFlags != D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY)
		resource.currentResourceState = resourceBarrierStateAfter;
}

void Graphics::ResourceManager::SetResourceBarrier(ID3D12GraphicsCommandList* _commandList, size_t swapChainBufferId, D3D12_RESOURCE_BARRIER_FLAGS resourceBarrierFlags,
	D3D12_RESOURCE_STATES resourceBarrierStateAfter)
{
	SetResourceBarrier(_commandList, swapChainBuffers[swapChainBufferId].Get(), resourceBarrierFlags, swapChainStates[swapChainBufferId], resourceBarrierStateAfter);

	if (resourceBarrierFlags != D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY)
		swapChainStates[swapChainBufferId] = resourceBarrierStateAfter;
}

void Graphics::ResourceManager::SetUAVBarrier(ID3D12GraphicsCommandList* _commandList, const RWTextureId& resourceId)
{
	SetUAVBarrier(_commandList, rwTexturePool[resourceId.value].textureAllocation.textureResource);
}

void Graphics::ResourceManager::SetUAVBarrier(ID3D12GraphicsCommandList* _commandList, const RWBufferId& resourceId)
{
	SetUAVBarrier(_commandList, rwBufferPool[resourceId.value].bufferAllocation.bufferResource);
}

void Graphics::ResourceManager::ReleaseTemporaryUploadBuffers()
{
	bufferAllocator.ReleaseTemporaryBuffers();
	textureAllocator.ReleaseTemporaryBuffers();
}

void Graphics::ResourceManager::GetTextureDataFromGPU(ID3D12Resource* resource, const TextureInfo& textureInfo, D3D12_RESOURCE_STATES beforeResourceState,
	std::vector<float4>& rawTextureData)
{
	uint64_t rowPitch = AlignSize(textureInfo.rowPitch, 256ui64);

	uint64_t requiredSize = rowPitch * textureInfo.height;

	BufferAllocation readbackTextureAllocation{};
	bufferAllocator.AllocateTemporary(device, requiredSize, D3D12_HEAP_TYPE_READBACK, readbackTextureAllocation);

	SetResourceBarrier(commandList.Get(), resource, D3D12_RESOURCE_BARRIER_FLAG_NONE, beforeResourceState, D3D12_RESOURCE_STATE_COPY_SOURCE);

	D3D12_TEXTURE_COPY_LOCATION srcLocation{};
	srcLocation.pResource = resource;

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT subresourceFootprint{};
	subresourceFootprint.Footprint.Width = static_cast<uint32_t>(textureInfo.width);
	subresourceFootprint.Footprint.Height = textureInfo.height;
	subresourceFootprint.Footprint.RowPitch = static_cast<uint32_t>(rowPitch);
	subresourceFootprint.Footprint.Depth = 1;
	subresourceFootprint.Footprint.Format = textureInfo.format;

	D3D12_TEXTURE_COPY_LOCATION destLocation{};
	destLocation.pResource = readbackTextureAllocation.bufferResource;
	destLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	destLocation.PlacedFootprint = subresourceFootprint;

	commandList->CopyTextureRegion(&destLocation, 0, 0, 0, &srcLocation, nullptr);

	SetResourceBarrier(commandList.Get(), resource, D3D12_RESOURCE_BARRIER_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_SOURCE, beforeResourceState);

	ExecuteGPUCommands();

	auto rawData = readbackTextureAllocation.cpuAddress;

	if (rawData == nullptr)
		throw std::exception("ResourceManager::GetTextureDataFromGPU: Getting the texture is failed");

	rawTextureData.resize(textureInfo.width * textureInfo.height, {});
	size_t texelId = 0;

	for (size_t pixelId = 0; pixelId < requiredSize / 4; pixelId++)
	{
		if (pixelId % (rowPitch / 4) >= textureInfo.rowPitch / 4)
			pixelId += (rowPitch - textureInfo.rowPitch) / 4;

		if (pixelId >= requiredSize / 4)
			break;

		float4 texel{};
		texel.x = rawData[pixelId * 4] / 255.0f;
		texel.y = rawData[pixelId * 4 + 1] / 255.0f;
		texel.z = rawData[pixelId * 4 + 2] / 255.0f;
		texel.w = rawData[pixelId * 4 + 3] / 255.0f;

		rawTextureData[texelId++] = texel;
	}
}

void Graphics::ResourceManager::GetBufferDataFromGPU(ID3D12Resource* resource, size_t requiredSize, D3D12_RESOURCE_STATES beforeResourceState, std::vector<uint8_t>& rawBufferData)
{
	BufferAllocation readbackBufferAllocation{};
	bufferAllocator.AllocateTemporary(device, requiredSize, D3D12_HEAP_TYPE_READBACK, readbackBufferAllocation);

	SetResourceBarrier(commandList.Get(), resource, D3D12_RESOURCE_BARRIER_FLAG_NONE, beforeResourceState, D3D12_RESOURCE_STATE_COPY_SOURCE);

	if (readbackBufferAllocation.bufferResource != nullptr)
		commandList->CopyResource(readbackBufferAllocation.bufferResource, resource);

	SetResourceBarrier(commandList.Get(), resource, D3D12_RESOURCE_BARRIER_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_SOURCE, beforeResourceState);

	ExecuteGPUCommands();

	auto rawData = readbackBufferAllocation.cpuAddress;

	if (rawData == nullptr)
		throw std::exception("ResourceManager::GetBufferDataFromGPU: Getting the buffer is failed");

	rawBufferData.resize(requiredSize, {});

	std::copy(rawData, rawData + requiredSize, rawBufferData.data());
}

void Graphics::ResourceManager::UploadTexture(ID3D12Resource* uploadBuffer, ID3D12Resource* targetTexture, const TextureInfo& textureInfo, const std::vector<uint8_t>& data,
	uint8_t* uploadBufferCPUAddress)
{
	uint32_t numSubresources = textureInfo.depth * textureInfo.mipLevels;

	std::vector<uint8_t> srcLayouts((sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(uint32_t) + sizeof(uint64_t)) * numSubresources);

	auto targetTextureDesc = targetTexture->GetDesc();
	std::vector<uint32_t> numRows(numSubresources);
	std::vector<uint64_t> rowSizesPerByte(numSubresources);
	uint64_t requiredSize;

	device->GetCopyableFootprints(&targetTextureDesc, 0, numSubresources, 0, reinterpret_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT*>(srcLayouts.data()), numRows.data(),
		rowSizesPerByte.data(), &requiredSize);

	for (uint32_t subresourceIndex = 0; subresourceIndex < numSubresources; subresourceIndex++)
	{
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT srcLayout = reinterpret_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT*>(&srcLayouts[0])[subresourceIndex];

		CopyRawDataToSubresource(textureInfo, numRows[subresourceIndex], srcLayout.Footprint.Depth, srcLayout.Footprint.RowPitch,
			static_cast<uint64_t>(srcLayout.Footprint.RowPitch) * numRows[subresourceIndex], rowSizesPerByte[subresourceIndex], &data[0] + srcLayout.Offset*0, uploadBufferCPUAddress);
	}
	
	for (uint32_t subresourceIndex = 0; subresourceIndex < numSubresources; subresourceIndex++)
	{
		D3D12_TEXTURE_COPY_LOCATION srcLocation{};
		srcLocation.pResource = uploadBuffer;
		srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		srcLocation.PlacedFootprint = reinterpret_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT*>(&srcLayouts[0])[subresourceIndex];

		D3D12_TEXTURE_COPY_LOCATION destLocation{};
		destLocation.pResource = targetTexture;
		destLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		destLocation.SubresourceIndex = subresourceIndex;

		commandList->CopyTextureRegion(&destLocation, 0, 0, 0, &srcLocation, nullptr);
	}
}

void Graphics::ResourceManager::CopyRawDataToSubresource(const TextureInfo& srcTextureInfo, uint32_t numRows, uint16_t numSlices, uint64_t destRowPitch,
	uint64_t destSlicePitch, uint64_t rowSizeInBytes, const uint8_t* srcAddress, uint8_t* destAddress)
{
	for (uint16_t sliceIndex = 0; sliceIndex < numSlices; sliceIndex++)
	{
		for (uint64_t rowIndex = 0; rowIndex < numRows; rowIndex++)
		{
			const uint8_t* srcBeginAddress = srcAddress + sliceIndex * srcTextureInfo.slicePitch + rowIndex * srcTextureInfo.rowPitch;
			const uint8_t* srcEndAddress = srcBeginAddress + rowSizeInBytes;
			uint8_t* destBeginAddress = destAddress + sliceIndex * destSlicePitch + rowIndex * destRowPitch;

			std::copy(srcBeginAddress, srcEndAddress, destBeginAddress);
		}
	}
}

void Graphics::ResourceManager::SetResourceBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* const resource, D3D12_RESOURCE_BARRIER_FLAGS resourceBarrierFlags,
	D3D12_RESOURCE_STATES resourceBarrierStateBefore, D3D12_RESOURCE_STATES resourceBarrierStateAfter)
{
	if (resourceBarrierStateBefore == resourceBarrierStateAfter)
		return;

	D3D12_RESOURCE_BARRIER resourceBarrier{};
	resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	resourceBarrier.Flags = resourceBarrierFlags;
	resourceBarrier.Transition.pResource = resource;
	resourceBarrier.Transition.StateBefore = resourceBarrierStateBefore;
	resourceBarrier.Transition.StateAfter = resourceBarrierStateAfter;
	resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	commandList->ResourceBarrier(1, &resourceBarrier);
}

void Graphics::ResourceManager::SetUAVBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* const resource)
{
	D3D12_RESOURCE_BARRIER resourceBarrier{};
	resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	resourceBarrier.UAV.pResource = resource;

	commandList->ResourceBarrier(1, &resourceBarrier);
}

void Graphics::ResourceManager::ExecuteGPUCommands()
{
	ThrowIfFailed(commandList->Close(), "ResourceManager::ExecuteGPUCommands: Command List closing error!");

	ID3D12CommandList* commandLists[] = { commandList.Get() };

	commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	ThrowIfFailed(commandQueue->Signal(fence.Get(), fenceValue), "ResourceManager::ExecuteGPUCommands: Signal error!");
	ThrowIfFailed(fence->SetEventOnCompletion(fenceValue, fenceEvent), "ResourceManager::ExecuteGPUCommands: Fence error!");

	WaitForSingleObjectEx(fenceEvent, INFINITE, false);

	fenceValue++;

	ThrowIfFailed(commandAllocator->Reset(), "ResourceManager::ExecuteGPUCommands: Command Allocator resetting error!");
	ThrowIfFailed(commandList->Reset(commandAllocator.Get(), nullptr), "ResourceManager::ExecuteGPUCommands: Command List resetting error!");
}
