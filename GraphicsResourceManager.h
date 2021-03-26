#pragma once

#include "GraphicsBufferAllocator.h"

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

struct GraphicsVertexBuffer
{
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	std::shared_ptr<GraphicsBufferAllocation> vertexBufferAllocation;
	std::shared_ptr<GraphicsBufferAllocation> uploadBufferAllocation;
};

class GraphicsResourceManager
{
public:
	static GraphicsResourceManager& GetInstance();

	void Initialize(ID3D12Device* _device, ID3D12GraphicsCommandList* _commandList);

	const VertexBufferId&& CreateVertexBuffer(std::vector<uint8_t>& data, uint32_t vertexStride);

	const GraphicsVertexBuffer& GetVertexBuffer(const VertexBufferId& resourceId);

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

	GraphicsBufferAllocator& bufferAllocator = GraphicsBufferAllocator::GetInstance();
};
