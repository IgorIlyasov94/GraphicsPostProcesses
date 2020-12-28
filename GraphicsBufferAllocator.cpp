#include "GraphicsBufferAllocator.h"

GraphicsBufferAllocator& GraphicsBufferAllocator::GetInstance()
{
	static GraphicsBufferAllocator thisInstance;

	return thisInstance;
}

void GraphicsBufferAllocator::Allocate(ID3D12Device* device, size_t size, size_t alignment, GraphicsBufferAllocation& allocation)
{
	if (size > pageSize)
		throw std::bad_alloc();

	if (!currentPage || !currentPage->HasSpace(size, alignment))
		SetNewPageAsCurrent(device, currentPage);

	currentPage->Allocate(size, alignment, allocation);
}

void GraphicsBufferAllocator::SetNewPageAsCurrent(ID3D12Device* device, std::shared_ptr<GraphicsBufferAllocationPage>& oldCurrentPage)
{
	if (emptyPages.empty())
	{
		GraphicsBufferAllocationPage newPage(device, pageSize);

		oldCurrentPage = std::make_shared<GraphicsBufferAllocationPage>(device, pageSize);

		usedPages.push_back(oldCurrentPage);
	}
	else
	{
		oldCurrentPage = emptyPages.front();

		emptyPages.pop_front();
	}
}
