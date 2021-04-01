#pragma once

#include "GraphicsHelper.h"
#include "GraphicsBufferAllocationPage.h"

class GraphicsBufferAllocator
{
public:
	static GraphicsBufferAllocator& GetInstance();

	void Allocate(ID3D12Device* device, size_t size, size_t alignment, D3D12_HEAP_TYPE heapType, GraphicsBufferAllocation& allocation);
	void AllocateTemporaryUpload(ID3D12Device* device, size_t size, GraphicsBufferAllocation& allocation);

	void ReleaseTemporaryBuffers();

private:
	GraphicsBufferAllocator() {};
	~GraphicsBufferAllocator() {};

	GraphicsBufferAllocator(const GraphicsBufferAllocator&) = delete;
	GraphicsBufferAllocator(GraphicsBufferAllocator&&) = delete;
	GraphicsBufferAllocator& operator=(const GraphicsBufferAllocator&) = delete;
	GraphicsBufferAllocator& operator=(GraphicsBufferAllocator&&) = delete;

	using BufferAllocationPagePool = std::deque<std::shared_ptr<GraphicsBufferAllocationPage>>;

	void Allocate(ID3D12Device* device, size_t size, size_t alignment, D3D12_HEAP_TYPE heapType, BufferAllocationPagePool& emptyPagePool,
		BufferAllocationPagePool& usedPagePool, std::shared_ptr<GraphicsBufferAllocationPage>& currentPage, GraphicsBufferAllocation& allocation);

	void SetNewPageAsCurrent(ID3D12Device* device, D3D12_HEAP_TYPE heapType, BufferAllocationPagePool& emptyPagePool,
		BufferAllocationPagePool& usedPagePool, std::shared_ptr<GraphicsBufferAllocationPage>& currentPage);
	
	BufferAllocationPagePool usedDefaultPages;
	BufferAllocationPagePool usedUploadPages;

	BufferAllocationPagePool emptyDefaultPages;
	BufferAllocationPagePool emptyUploadPages;
	
	std::shared_ptr<GraphicsBufferAllocationPage> currentDefaultPage;
	std::shared_ptr<GraphicsBufferAllocationPage> currentUploadPage;

	BufferAllocationPagePool tempUploadPages;

	const size_t pageSize = 2 * _MB;
};
