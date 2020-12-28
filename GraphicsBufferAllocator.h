#pragma once

#include "GraphicsHelper.h"
#include "GraphicsBufferAllocationPage.h"

class GraphicsBufferAllocator
{
public:
	static GraphicsBufferAllocator& GetInstance();

	void Allocate(ID3D12Device* device, size_t size, size_t alignment, GraphicsBufferAllocation& allocation);

private:
	GraphicsBufferAllocator() {};
	~GraphicsBufferAllocator() {};

	GraphicsBufferAllocator(const GraphicsBufferAllocator&) = delete;
	GraphicsBufferAllocator(GraphicsBufferAllocator&&) = delete;
	GraphicsBufferAllocator& operator=(const GraphicsBufferAllocator&) = delete;
	GraphicsBufferAllocator& operator=(GraphicsBufferAllocator&&) = delete;

	void SetNewPageAsCurrent(ID3D12Device* device, std::shared_ptr<GraphicsBufferAllocationPage>& oldCurrentPage);

	std::deque<std::shared_ptr<GraphicsBufferAllocationPage>> usedPages;
	std::deque<std::shared_ptr<GraphicsBufferAllocationPage>> emptyPages;

	std::shared_ptr<GraphicsBufferAllocationPage> currentPage;

	const size_t pageSize = 64 * 1024;
};