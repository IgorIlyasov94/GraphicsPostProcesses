#include "GraphicsResourceManager.h"

GraphicsResourceManager& GraphicsResourceManager::GetInstance()
{
	static GraphicsResourceManager thisInstance;

	return thisInstance;
}

void GraphicsResourceManager::Initialize(ID3D12Device* _device, ID3D12GraphicsCommandList* _commandList)
{
	device = _device;
	commandList = _commandList;
}

const VertexBufferId&& GraphicsResourceManager::CreateVertexBuffer(std::vector<uint8_t>& data, uint32_t vertexStride)
{
	GraphicsBufferAllocation vertexBufferAllocation{};

	bufferAllocator.AllocateDefault(device, data.size(), 64 * _KB, vertexBufferAllocation);

	GraphicsBufferAllocation uploadBufferAllocation{};

	bufferAllocator.AllocateUpload(device, data.size(), 64 * _KB, uploadBufferAllocation);

	std::copy(data.begin(), data.end(), uploadBufferAllocation.cpuAddress);

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};

	vertexBufferView.BufferLocation = vertexBufferAllocation.gpuAddress;
	vertexBufferView.SizeInBytes = data.size();
	vertexBufferView.StrideInBytes = vertexStride;

	ThrowIfFailed(vertexBufferAllocation.bufferResource == nullptr,
		"GraphicsResourceManager::CreateVertexBuffer: Vertex Buffer Resource is null!");

	ThrowIfFailed(uploadBufferAllocation.bufferResource == nullptr,
		"GraphicsResourceManager::CreateVertexBuffer: Upload Buffer Resource is null!");

	commandList->CopyBufferRegion(vertexBufferAllocation.bufferResource, 0, uploadBufferAllocation.bufferResource, 0,
		vertexBufferAllocation.bufferResource->GetDesc().Width);

	SetResourceBarrier(commandList, vertexBufferAllocation.bufferResource, D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	GraphicsVertexBuffer graphicsVertexBuffer;

	graphicsVertexBuffer.vertexBufferAllocation = std::make_shared<GraphicsBufferAllocation>(vertexBufferAllocation);
	graphicsVertexBuffer.uploadBufferAllocation = std::make_shared<GraphicsBufferAllocation>(uploadBufferAllocation);
	graphicsVertexBuffer.vertexBufferView.BufferLocation = vertexBufferAllocation.gpuAddress;
	graphicsVertexBuffer.vertexBufferView.SizeInBytes = data.size();
	graphicsVertexBuffer.vertexBufferView.StrideInBytes = vertexStride;

	vertexBufferPool.push_back(graphicsVertexBuffer);

	return std::move(VertexBufferId(vertexBufferPool.size() - 1));
}

const GraphicsVertexBuffer& GraphicsResourceManager::GetVertexBuffer(const VertexBufferId& resourceId)
{
	return vertexBufferPool[resourceId.value];
}
