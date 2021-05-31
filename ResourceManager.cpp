#include "ResourceManager.h"

Graphics::ResourceManager& Graphics::ResourceManager::GetInstance()
{
	static ResourceManager thisInstance;

	return thisInstance;
}

void Graphics::ResourceManager::Initialize(ID3D12Device* _device, ID3D12GraphicsCommandList* _commandList)
{
	device = _device;
	commandList = _commandList;
}

Graphics::VertexBufferId Graphics::ResourceManager::CreateVertexBuffer(const void* data, size_t dataSize, uint32_t vertexStride)
{
	BufferAllocation vertexBufferAllocation{};
	bufferAllocator.Allocate(device, dataSize, 64 * _KB, D3D12_HEAP_TYPE_DEFAULT, vertexBufferAllocation);

	BufferAllocation uploadBufferAllocation{};
	bufferAllocator.AllocateTemporaryUpload(device, dataSize, uploadBufferAllocation);

	std::copy(reinterpret_cast<const uint8_t*>(data), reinterpret_cast<const uint8_t*>(data) + dataSize, uploadBufferAllocation.cpuAddress);

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView.BufferLocation = vertexBufferAllocation.gpuAddress;
	vertexBufferView.SizeInBytes = dataSize;
	vertexBufferView.StrideInBytes = vertexStride;

	if (vertexBufferAllocation.bufferResource == nullptr)
		throw std::exception("ResourceManager::CreateVertexBuffer: Vertex Buffer Resource is null!");

	if (uploadBufferAllocation.bufferResource == nullptr)
		throw std::exception("ResourceManager::CreateVertexBuffer: Upload Buffer Resource is null!");

	commandList->CopyBufferRegion(vertexBufferAllocation.bufferResource, vertexBufferAllocation.gpuPageOffset, uploadBufferAllocation.bufferResource,
		0, uploadBufferAllocation.bufferResource->GetDesc().Width);

	SetResourceBarrier(commandList, vertexBufferAllocation.bufferResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	VertexBuffer vertexBuffer{};
	vertexBuffer.vertexBufferAllocation = vertexBufferAllocation;
	vertexBuffer.vertexBufferView = vertexBufferView;

	vertexBufferPool.push_back(vertexBuffer);

	return VertexBufferId(vertexBufferPool.size() - 1);
}

Graphics::IndexBufferId Graphics::ResourceManager::CreateIndexBuffer(const void* data, size_t dataSize, uint32_t indexStride)
{
	BufferAllocation indexBufferAllocation{};
	bufferAllocator.Allocate(device, dataSize, 64 * _KB, D3D12_HEAP_TYPE_DEFAULT, indexBufferAllocation);
	BufferAllocation uploadBufferAllocation{};

	bufferAllocator.AllocateTemporaryUpload(device, dataSize, uploadBufferAllocation);

	std::copy(reinterpret_cast<const uint8_t*>(data), reinterpret_cast<const uint8_t*>(data) + dataSize, uploadBufferAllocation.cpuAddress);

	D3D12_INDEX_BUFFER_VIEW indexBufferView{};
	indexBufferView.BufferLocation = indexBufferAllocation.gpuAddress;
	indexBufferView.SizeInBytes = dataSize;
	indexBufferView.Format = (indexStride == 4) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;

	if (indexBufferAllocation.bufferResource == nullptr)
		throw std::exception("ResourceManager::CreateIndexBuffer: Index Buffer Resource is null!");

	if (uploadBufferAllocation.bufferResource == nullptr)
		throw std::exception("ResourceManager::CreateIndexBuffer: Upload Buffer Resource is null!");

	commandList->CopyBufferRegion(indexBufferAllocation.bufferResource, indexBufferAllocation.gpuPageOffset, uploadBufferAllocation.bufferResource,
		0, uploadBufferAllocation.bufferResource->GetDesc().Width);

	SetResourceBarrier(commandList, indexBufferAllocation.bufferResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);

	IndexBuffer indexBuffer{};
	indexBuffer.indicesCount = dataSize / indexStride;
	indexBuffer.indexBufferView = indexBufferView;
	indexBuffer.indexBufferAllocation = indexBufferAllocation;
	
	indexBufferPool.push_back(indexBuffer);

	return IndexBufferId(indexBufferPool.size() - 1);
}

Graphics::ConstantBufferId Graphics::ResourceManager::CreateConstantBuffer(const void* data, size_t dataSize)
{
	BufferAllocation constantBufferAllocation{};
	bufferAllocator.Allocate(device, dataSize, 64 * _KB, D3D12_HEAP_TYPE_UPLOAD, constantBufferAllocation);

	DescriptorAllocation constantBufferDescriptorAllocation{};
	descriptorAllocator.Allocate(device, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, constantBufferDescriptorAllocation);

	D3D12_CONSTANT_BUFFER_VIEW_DESC constantBufferViewDesc{};
	constantBufferViewDesc.BufferLocation = constantBufferAllocation.gpuAddress;
	constantBufferViewDesc.SizeInBytes = AlignSize(dataSize, static_cast<size_t>(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT));

	device->CreateConstantBufferView(&constantBufferViewDesc, constantBufferDescriptorAllocation.descriptorBase);

	std::copy(reinterpret_cast<const uint8_t*>(data), reinterpret_cast<const uint8_t*>(data) + dataSize, constantBufferAllocation.cpuAddress);

	ConstantBuffer constantBuffer{};
	constantBuffer.uploadBufferAllocation = constantBufferAllocation;
	constantBuffer.bufferDescriptorAllocation = constantBufferDescriptorAllocation;
	constantBuffer.constantBufferViewDesc = constantBufferViewDesc;

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

	SetResourceBarrier(commandList, textureAllocation.textureResource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);

	UploadTexture(uploadTextureAllocation.textureResource, textureAllocation.textureResource, textureInfo, data, uploadTextureAllocation.cpuAddress);

	SetResourceBarrier(commandList, textureAllocation.textureResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);

	DescriptorAllocation shaderResourceDescriptorAllocation{};
	descriptorAllocator.Allocate(device, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shaderResourceDescriptorAllocation);

	device->CreateShaderResourceView(textureAllocation.textureResource, &shaderResourceViewDesc, shaderResourceDescriptorAllocation.descriptorBase);

	Texture texture{};
	texture.shaderResourceViewDesc = shaderResourceViewDesc;
	texture.info = textureInfo;
	texture.textureAllocation = textureAllocation;
	texture.shaderResourceDescriptorAllocation = shaderResourceDescriptorAllocation;

	texturePool.push_back(texture);

	return TextureId(texturePool.size() - 1);
}

Graphics::BufferId Graphics::ResourceManager::CreateBuffer(const void* data, size_t dataSize, uint32_t bufferStride, uint32_t numElements, DXGI_FORMAT format)
{
	BufferAllocation bufferAllocation{};
	bufferAllocator.AllocateUnorderedAccess(device, dataSize, 64 * _KB, bufferAllocation);

	BufferAllocation uploadBufferAllocation{};
	bufferAllocator.AllocateTemporaryUpload(device, dataSize, uploadBufferAllocation);

	std::copy(reinterpret_cast<const uint8_t*>(data), reinterpret_cast<const uint8_t*>(data) + dataSize, uploadBufferAllocation.cpuAddress);

	if (bufferAllocation.bufferResource == nullptr)
		throw std::exception("ResourceManager::CreateBuffer: Buffer Resource is null!");

	if (uploadBufferAllocation.bufferResource == nullptr)
		throw std::exception("ResourceManager::CreateBuffer: Upload Buffer Resource is null!");

	commandList->CopyBufferRegion(bufferAllocation.bufferResource, bufferAllocation.gpuPageOffset, uploadBufferAllocation.bufferResource,
		0, uploadBufferAllocation.bufferResource->GetDesc().Width);

	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	shaderResourceViewDesc.Format = (bufferStride == 0) ? format : (bufferStride == 1) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_UNKNOWN;
	shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceViewDesc.Buffer.Flags = (bufferStride == 1) ? D3D12_BUFFER_SRV_FLAG_RAW : D3D12_BUFFER_SRV_FLAG_NONE;
	shaderResourceViewDesc.Buffer.NumElements = numElements;
	shaderResourceViewDesc.Buffer.StructureByteStride = bufferStride;

	DescriptorAllocation shaderResourceDescriptorAllocation{};
	descriptorAllocator.Allocate(device, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shaderResourceDescriptorAllocation);
	device->CreateShaderResourceView(bufferAllocation.bufferResource, &shaderResourceViewDesc, shaderResourceDescriptorAllocation.descriptorBase);

	Buffer buffer{};
	buffer.bufferAllocation = bufferAllocation;
	buffer.shaderResourceViewDesc = shaderResourceViewDesc;
	buffer.shaderResourceDescriptorAllocation = shaderResourceDescriptorAllocation;

	bufferPool.push_back(buffer);

	return BufferId(bufferPool.size() - 1);
}

Graphics::SamplerId Graphics::ResourceManager::CreateSampler(const D3D12_SAMPLER_DESC& samplerDesc)
{
	DescriptorAllocation samplerDescriptorAllocation{};
	descriptorAllocator.Allocate(device, 1, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, samplerDescriptorAllocation);

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
	descriptorAllocator.Allocate(device, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shaderResourceDescriptorAllocation);
	device->CreateShaderResourceView(textureAllocation.textureResource, &shaderResourceViewDesc, shaderResourceDescriptorAllocation.descriptorBase);

	D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc{};
	renderTargetViewDesc.Format = format;
	renderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	
	DescriptorAllocation renderTargetDescriptorAllocation{};
	descriptorAllocator.Allocate(device, 1, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, renderTargetDescriptorAllocation);

	device->CreateRenderTargetView(textureAllocation.textureResource, &renderTargetViewDesc, renderTargetDescriptorAllocation.descriptorBase);

	SetResourceBarrier(commandList, textureAllocation.textureResource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);

	RenderTarget renderTarget{};
	renderTarget.shaderResourceViewDesc = shaderResourceViewDesc;
	renderTarget.renderTargetViewDesc = renderTargetViewDesc;
	renderTarget.info = textureInfo;
	renderTarget.textureAllocation = textureAllocation;
	renderTarget.renderTargetDescriptorAllocation = renderTargetDescriptorAllocation;
	renderTarget.shaderResourceDescriptorAllocation = shaderResourceDescriptorAllocation;

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
	descriptorAllocator.Allocate(device, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shaderResourceDescriptorAllocation);
	device->CreateShaderResourceView(textureAllocation.textureResource, &shaderResourceViewDesc, shaderResourceDescriptorAllocation.descriptorBase);

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = (depthBit == 32) ? DXGI_FORMAT_D32_FLOAT : DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	DescriptorAllocation depthStencilDescriptorAllocation{};
	descriptorAllocator.Allocate(device, 1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, depthStencilDescriptorAllocation);
	device->CreateDepthStencilView(textureAllocation.textureResource, &depthStencilViewDesc, depthStencilDescriptorAllocation.descriptorBase);

	SetResourceBarrier(commandList, textureAllocation.textureResource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);

	DepthStencil depthStencil{};
	depthStencil.shaderResourceViewDesc = shaderResourceViewDesc;
	depthStencil.depthStencilViewDesc = depthStencilViewDesc;
	depthStencil.info = textureInfo;
	depthStencil.textureAllocation = textureAllocation;
	depthStencil.depthStencilDescriptorAllocation = depthStencilDescriptorAllocation;
	depthStencil.shaderResourceDescriptorAllocation = shaderResourceDescriptorAllocation;

	depthStencilPool.push_back(depthStencil);

	return DepthStencilId(depthStencilPool.size() - 1);
}

Graphics::RWTextureId Graphics::ResourceManager::CreateRWTexture(const TextureInfo& textureInfo, D3D12_RESOURCE_FLAGS resourceFlags)
{
	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = textureInfo.format;

	TextureAllocation textureAllocation{};
	textureAllocator.Allocate(device, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, &clearValue, textureInfo, textureAllocation);

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
	descriptorAllocator.Allocate(device, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shaderResourceDescriptorAllocation);
	device->CreateShaderResourceView(textureAllocation.textureResource, &shaderResourceViewDesc, shaderResourceDescriptorAllocation.descriptorBase);

	DescriptorAllocation unorderedAccessDescriptorAllocation{};
	descriptorAllocator.Allocate(device, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, unorderedAccessDescriptorAllocation);
	device->CreateUnorderedAccessView(textureAllocation.textureResource, nullptr, &unorderedAccessViewDesc, unorderedAccessDescriptorAllocation.descriptorBase);

	SetResourceBarrier(commandList, textureAllocation.textureResource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	RWTexture rwTexture{};
	rwTexture.shaderResourceViewDesc = shaderResourceViewDesc;
	rwTexture.unorderedAccessViewDesc = unorderedAccessViewDesc;
	rwTexture.info = textureInfo;
	rwTexture.textureAllocation = textureAllocation;
	rwTexture.unorderedAccessDescriptorAllocation = unorderedAccessDescriptorAllocation;
	rwTexture.shaderResourceDescriptorAllocation = shaderResourceDescriptorAllocation;

	rwTexturePool.push_back(rwTexture);

	return RWTextureId(rwTexturePool.size() - 1);
}

Graphics::RWBufferId Graphics::ResourceManager::CreateRWBuffer(const void* initialData, size_t dataSize, uint32_t bufferStride, uint32_t numElements, DXGI_FORMAT format)
{
	BufferAllocation bufferAllocation{};
	bufferAllocator.AllocateUnorderedAccess(device, dataSize, 64 * _KB, bufferAllocation);

	BufferAllocation uploadBufferAllocation{};
	bufferAllocator.AllocateTemporaryUpload(device, dataSize, uploadBufferAllocation);

	std::copy(reinterpret_cast<const uint8_t*>(initialData), reinterpret_cast<const uint8_t*>(initialData) + dataSize, uploadBufferAllocation.cpuAddress);

	if (bufferAllocation.bufferResource == nullptr)
		throw std::exception("ResourceManager::CreateRWBuffer: RWBuffer Resource is null!");

	if (uploadBufferAllocation.bufferResource == nullptr)
		throw std::exception("ResourceManager::CreateRWBuffer: Upload Buffer Resource is null!");

	commandList->CopyBufferRegion(bufferAllocation.bufferResource, bufferAllocation.gpuPageOffset, uploadBufferAllocation.bufferResource,
		0, uploadBufferAllocation.bufferResource->GetDesc().Width);

	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	shaderResourceViewDesc.Format = (bufferStride == 0) ? format : (bufferStride == 1) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_UNKNOWN;
	shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceViewDesc.Buffer.Flags = (bufferStride == 1) ? D3D12_BUFFER_SRV_FLAG_RAW : D3D12_BUFFER_SRV_FLAG_NONE;
	shaderResourceViewDesc.Buffer.NumElements = numElements;
	shaderResourceViewDesc.Buffer.StructureByteStride = (bufferStride > 1) ? bufferStride : 0;

	DescriptorAllocation shaderResourceDescriptorAllocation{};
	descriptorAllocator.Allocate(device, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shaderResourceDescriptorAllocation);
	device->CreateShaderResourceView(bufferAllocation.bufferResource, &shaderResourceViewDesc, shaderResourceDescriptorAllocation.descriptorBase);

	D3D12_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc{};
	unorderedAccessViewDesc.Format = (bufferStride == 0) ? format : (bufferStride == 1) ? DXGI_FORMAT_R32_TYPELESS : DXGI_FORMAT_UNKNOWN;
	unorderedAccessViewDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	unorderedAccessViewDesc.Buffer.Flags = (bufferStride == 1) ? D3D12_BUFFER_UAV_FLAG_RAW : D3D12_BUFFER_UAV_FLAG_NONE;
	unorderedAccessViewDesc.Buffer.NumElements = numElements;
	unorderedAccessViewDesc.Buffer.StructureByteStride = (bufferStride > 1) ? bufferStride : 0;

	DescriptorAllocation unorderedAccessDescriptorAllocation{};
	descriptorAllocator.Allocate(device, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, unorderedAccessDescriptorAllocation);

	if (bufferStride <= 1)
		device->CreateUnorderedAccessView(bufferAllocation.bufferResource, nullptr, &unorderedAccessViewDesc, unorderedAccessDescriptorAllocation.descriptorBase);
	else
	{
		BufferAllocation counterBufferAllocation{};
		bufferAllocator.AllocateUnorderedAccess(device, 4, 64 * _KB, counterBufferAllocation);

		device->CreateUnorderedAccessView(bufferAllocation.bufferResource, counterBufferAllocation.bufferResource, &unorderedAccessViewDesc,
			unorderedAccessDescriptorAllocation.descriptorBase);
	}

	RWBuffer rwBuffer{};
	rwBuffer.bufferAllocation = bufferAllocation;
	rwBuffer.shaderResourceViewDesc = shaderResourceViewDesc;
	rwBuffer.unorderedAccessViewDesc = unorderedAccessViewDesc;
	rwBuffer.unorderedAccessDescriptorAllocation = unorderedAccessDescriptorAllocation;
	rwBuffer.shaderResourceDescriptorAllocation = shaderResourceDescriptorAllocation;
	
	rwBufferPool.push_back(rwBuffer);

	return RWBufferId(rwBufferPool.size() - 1);
}

void Graphics::ResourceManager::CreateSwapChainBuffers(IDXGISwapChain4* swapChain, uint32_t buffersCount)
{
	for (uint32_t bufferId = 0; bufferId < buffersCount; bufferId++)
	{
		DescriptorAllocation renderTargetDescriptorAllocation{};

		descriptorAllocator.Allocate(device, buffersCount, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, renderTargetDescriptorAllocation);
		swapChainDescriptorBases.push_back(renderTargetDescriptorAllocation.descriptorBase);

		swapChainBuffers.push_back(nullptr);
		swapChain->GetBuffer(bufferId, IID_PPV_ARGS(&swapChainBuffers.back()));

		device->CreateRenderTargetView(swapChainBuffers.back().Get(), nullptr, swapChainDescriptorBases.back());
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

	ThrowIfFailed(swapChain->ResizeBuffers(swapChainBuffers.size(), swapChainDesc.Width, swapChainDesc.Height, swapChainDesc.Format, swapChainDesc.Flags),
		"ResourceManager::ResetSwapChainBuffers: Back buffers resizing error!");

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

void Graphics::ResourceManager::UpdateConstantBuffer(const ConstantBufferId& resourceId, const void* data, size_t dataSize)
{
	std::copy(reinterpret_cast<const uint8_t*>(data), reinterpret_cast<const uint8_t*>(data) + dataSize,
		constantBufferPool[resourceId.value].uploadBufferAllocation.cpuAddress);
}

void Graphics::ResourceManager::ReleaseTemporaryUploadBuffers()
{
	bufferAllocator.ReleaseTemporaryBuffers();
	textureAllocator.ReleaseTemporaryBuffers();
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
