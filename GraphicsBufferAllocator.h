#pragma once

#include "GraphicsHelper.h"
#include "GraphicsBufferAllocationPage.h"

class GraphicsBufferAllocator
{
public:
	static GraphicsBufferAllocator& GetInstance();

	void AllocateDefault(ID3D12Device* device, size_t size, size_t alignment, GraphicsBufferAllocation& allocation);
	void AllocateUpload(ID3D12Device* device, size_t size, size_t alignment, GraphicsBufferAllocation& allocation);

private:
	GraphicsBufferAllocator() {};
	~GraphicsBufferAllocator() {};

	GraphicsBufferAllocator(const GraphicsBufferAllocator&) = delete;
	GraphicsBufferAllocator(GraphicsBufferAllocator&&) = delete;
	GraphicsBufferAllocator& operator=(const GraphicsBufferAllocator&) = delete;
	GraphicsBufferAllocator& operator=(GraphicsBufferAllocator&&) = delete;

	using DefaultBufferAllocation = GraphicsBufferAllocationPage<D3D12_HEAP_TYPE_DEFAULT>;
	using UploadBufferAllocation = GraphicsBufferAllocationPage<D3D12_HEAP_TYPE_UPLOAD>;

	void SetNewDefaultPageAsCurrent(ID3D12Device* device, std::shared_ptr<DefaultBufferAllocation>& oldCurrentPage);
	void SetNewUploadPageAsCurrent(ID3D12Device* device, std::shared_ptr<UploadBufferAllocation>& oldCurrentPage);

	std::deque<std::shared_ptr<DefaultBufferAllocation>> usedDefaultPages;
	std::deque<std::shared_ptr<UploadBufferAllocation>> usedUploadPages;

	std::deque<std::shared_ptr<DefaultBufferAllocation>> emptyDefaultPages;
	std::deque<std::shared_ptr<UploadBufferAllocation>> emptyUploadPages;
	
	std::shared_ptr<DefaultBufferAllocation> currentDefaultPage;
	std::shared_ptr<UploadBufferAllocation> currentUploadPage;

	const size_t pageSize = 2 * _MB;
};
