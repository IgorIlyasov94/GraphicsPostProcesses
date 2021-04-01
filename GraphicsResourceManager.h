#pragma once

#include "GraphicsBufferAllocator.h"
#include "GraphicsDescriptorAllocator.h"
#include "GraphicsTextureAllocator.h"
#include "GraphicsDDSLoader.h"

template<uint8_t Category>
class ResourceId
{
public:
	ResourceId()
		: value(0), category(Category)
	{

	}

	ResourceId(uint32_t resourceId)
		: value(resourceId), category(Category)
	{

	}

private:
	friend class GraphicsResourceManager;

	uint32_t value;
	uint8_t category;
};

using VertexBufferId = typename ResourceId<0>;
using ConstantBufferId = typename ResourceId<1>;
using TextureId = typename ResourceId<2>;
using SamplerId = typename ResourceId<3>;

struct GraphicsVertexBuffer
{
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	GraphicsBufferAllocation vertexBufferAllocation;
};

struct GraphicsConstantBuffer
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC constantBufferViewDesc;
	GraphicsBufferAllocation uploadBufferAllocation;
	GraphicsDescriptorAllocation descriptorAllocation;
};

struct GraphicsTexture
{
	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	TextureInfo info;
	GraphicsTextureAllocation textureAllocation;
	GraphicsDescriptorAllocation descriptorAllocation;
};

struct GraphicsSampler
{
	D3D12_SAMPLER_DESC samplerDesc;
	GraphicsDescriptorAllocation descriptorAllocation;
};

class GraphicsResourceManager
{
public:
	static GraphicsResourceManager& GetInstance();

	void Initialize(ID3D12Device* _device, ID3D12GraphicsCommandList* _commandList);

	VertexBufferId&& CreateVertexBuffer(std::vector<uint8_t>& data, uint32_t vertexStride);
	ConstantBufferId&& CreateConstantBuffer(std::vector<uint8_t>& data);
	TextureId&& CreateTexture(const std::filesystem::path& fileName);
	TextureId&& CreateTexture(const std::vector<uint8_t>& data, const TextureInfo& textureInfo, D3D12_RESOURCE_FLAGS resourceFlags);
	SamplerId&& CreateSampler(const D3D12_SAMPLER_DESC& samplerDesc);

	const GraphicsVertexBuffer& GetVertexBuffer(const VertexBufferId& resourceId);
	const GraphicsConstantBuffer& GetConstantBuffer(const ConstantBufferId& resourceId);
	const GraphicsTexture& GetTexture(const TextureId& resourceId);
	const GraphicsSampler& GetSampler(const SamplerId& resourceId);

	void ReleaseTemporaryUploadBuffers();

private:
	GraphicsResourceManager() : device(nullptr), commandList(nullptr) {};
	~GraphicsResourceManager() {};

	GraphicsResourceManager(const GraphicsResourceManager&) = delete;
	GraphicsResourceManager(GraphicsResourceManager&&) = delete;
	GraphicsResourceManager& operator=(const GraphicsResourceManager&) = delete;
	GraphicsResourceManager& operator=(GraphicsResourceManager&&) = delete;

	ID3D12Device* device;
	ID3D12GraphicsCommandList* commandList;

	std::vector<GraphicsVertexBuffer> vertexBufferPool;
	std::vector<GraphicsConstantBuffer> constantBufferPool;
	std::vector<GraphicsTexture> texturePool;
	std::vector<GraphicsSampler> samplerPool;

	GraphicsBufferAllocator& bufferAllocator = GraphicsBufferAllocator::GetInstance();
	GraphicsDescriptorAllocator& descriptorAllocator = GraphicsDescriptorAllocator::GetInstance();
	GraphicsTextureAllocator& textureAllocator = GraphicsTextureAllocator::GetInstance();
};
