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

Graphics::VertexBufferId Graphics::ResourceManager::CreateVertexBuffer(std::vector<uint8_t>& data, uint32_t vertexStride)
{
	BufferAllocation vertexBufferAllocation{};

	bufferAllocator.Allocate(device, data.size(), 64 * _KB, D3D12_HEAP_TYPE_DEFAULT, vertexBufferAllocation);

	BufferAllocation uploadBufferAllocation{};

	bufferAllocator.AllocateTemporaryUpload(device, data.size(), uploadBufferAllocation);

	std::copy(data.begin(), data.end(), uploadBufferAllocation.cpuAddress);

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView.BufferLocation = vertexBufferAllocation.gpuAddress;
	vertexBufferView.SizeInBytes = data.size();
	vertexBufferView.StrideInBytes = vertexStride;

	if (vertexBufferAllocation.bufferResource == nullptr)
		throw std::exception("ResourceManager::CreateVertexBuffer: Vertex Buffer Resource is null!");

	if (uploadBufferAllocation.bufferResource == nullptr)
		throw std::exception("ResourceManager::CreateVertexBuffer: Upload Buffer Resource is null!");

	commandList->CopyBufferRegion(vertexBufferAllocation.bufferResource, vertexBufferAllocation.gpuPageOffset, uploadBufferAllocation.bufferResource,
		0, uploadBufferAllocation.bufferResource->GetDesc().Width);

	SetResourceBarrier(commandList, vertexBufferAllocation.bufferResource, D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	VertexBuffer graphicsVertexBuffer{};
	graphicsVertexBuffer.vertexBufferAllocation = vertexBufferAllocation;
	graphicsVertexBuffer.vertexBufferView = vertexBufferView;

	vertexBufferPool.push_back(graphicsVertexBuffer);

	return VertexBufferId(vertexBufferPool.size() - 1);
}

Graphics::ConstantBufferId Graphics::ResourceManager::CreateConstantBuffer(std::vector<uint8_t>& data)
{
	BufferAllocation constantBufferAllocation{};

	bufferAllocator.Allocate(device, data.size(), 64 * _KB, D3D12_HEAP_TYPE_UPLOAD, constantBufferAllocation);

	DescriptorAllocation constantBufferDescriptorAllocation{};

	descriptorAllocator.Allocate(device, 1, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, constantBufferDescriptorAllocation);

	D3D12_CONSTANT_BUFFER_VIEW_DESC constantBufferViewDesc{};
	constantBufferViewDesc.BufferLocation = constantBufferAllocation.gpuAddress;
	constantBufferViewDesc.SizeInBytes = AlignSize(data.size(), static_cast<size_t>(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT));

	device->CreateConstantBufferView(&constantBufferViewDesc, constantBufferDescriptorAllocation.descriptorBase);

	std::copy(data.begin(), data.end(), constantBufferAllocation.cpuAddress);

	ConstantBuffer graphicsConstantBuffer{};
	graphicsConstantBuffer.uploadBufferAllocation = constantBufferAllocation;
	graphicsConstantBuffer.descriptorAllocation = constantBufferDescriptorAllocation;
	graphicsConstantBuffer.constantBufferViewDesc = constantBufferViewDesc;

	constantBufferPool.push_back(graphicsConstantBuffer);

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

	textureAllocator.Allocate(device, resourceFlags, textureInfo, textureAllocation);

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

	Texture graphicsTexture{};
	graphicsTexture.shaderResourceViewDesc = shaderResourceViewDesc;
	graphicsTexture.info = textureInfo;
	graphicsTexture.textureAllocation = textureAllocation;
	graphicsTexture.descriptorAllocation = shaderResourceDescriptorAllocation;

	texturePool.push_back(graphicsTexture);

	return TextureId(texturePool.size() - 1);
}

Graphics::SamplerId Graphics::ResourceManager::CreateSampler(const D3D12_SAMPLER_DESC& samplerDesc)
{
	DescriptorAllocation samplerDescriptorAllocation{};

	descriptorAllocator.Allocate(device, 1, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, samplerDescriptorAllocation);

	device->CreateSampler(&samplerDesc, samplerDescriptorAllocation.descriptorBase);

	Sampler graphicsSampler{};
	graphicsSampler.samplerDesc = samplerDesc;
	graphicsSampler.descriptorAllocation = samplerDescriptorAllocation;

	samplerPool.push_back(graphicsSampler);

	return SamplerId(samplerPool.size() - 1);
}

const Graphics::VertexBuffer& Graphics::ResourceManager::GetVertexBuffer(const VertexBufferId& resourceId)
{
	return vertexBufferPool[resourceId.value];
}

const Graphics::ConstantBuffer& Graphics::ResourceManager::GetConstantBuffer(const ConstantBufferId& resourceId)
{
	return constantBufferPool[resourceId.value];
}

const Graphics::Texture& Graphics::ResourceManager::GetTexture(const TextureId& resourceId)
{
	return texturePool[resourceId.value];
}

const Graphics::Sampler& Graphics::ResourceManager::GetSampler(const SamplerId& resourceId)
{
	return samplerPool[resourceId.value];
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
