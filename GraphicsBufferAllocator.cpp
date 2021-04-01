#include "GraphicsBufferAllocator.h"

GraphicsBufferAllocator& GraphicsBufferAllocator::GetInstance()
{
	static GraphicsBufferAllocator thisInstance;

	return thisInstance;
}

void GraphicsBufferAllocator::Allocate(ID3D12Device* device, size_t size, size_t alignment, D3D12_HEAP_TYPE heapType, GraphicsBufferAllocation& allocation)
{
	if (size > pageSize)
		throw std::exception("GraphicsBufferAllocator::Allocate: Bad allocation");

	if (heapType == D3D12_HEAP_TYPE_DEFAULT)
	{
		Allocate(device, size, alignment, heapType, emptyDefaultPages, usedDefaultPages, currentDefaultPage, allocation);
	}
	else if (heapType == D3D12_HEAP_TYPE_UPLOAD)
	{
		Allocate(device, size, alignment, heapType, emptyUploadPages, usedUploadPages, currentUploadPage, allocation);
	}
}

void GraphicsBufferAllocator::AllocateTemporaryUpload(ID3D12Device* device, size_t size, GraphicsBufferAllocation& allocation)
{
	tempUploadPages.push_back(std::make_shared<GraphicsBufferAllocationPage>(device, D3D12_HEAP_TYPE_UPLOAD, size));

	tempUploadPages[tempUploadPages.size() - 1]->Allocate(size, 1, allocation);
}

void GraphicsBufferAllocator::ReleaseTemporaryBuffers()
{
	tempUploadPages.clear();
}

void GraphicsBufferAllocator::Allocate(ID3D12Device* device, size_t size, size_t alignment, D3D12_HEAP_TYPE heapType, BufferAllocationPagePool& emptyPagePool,
	BufferAllocationPagePool& usedPagePool, std::shared_ptr<GraphicsBufferAllocationPage>& currentPage, GraphicsBufferAllocation& allocation)
{
	if (!currentPage || !currentPage->HasSpace(size, alignment))
		SetNewPageAsCurrent(device, heapType, emptyPagePool, usedPagePool, currentPage);

	currentPage->Allocate(size, alignment, allocation);
}

void GraphicsBufferAllocator::SetNewPageAsCurrent(ID3D12Device* device, D3D12_HEAP_TYPE heapType, BufferAllocationPagePool& emptyPagePool,
	BufferAllocationPagePool& usedPagePool, std::shared_ptr<GraphicsBufferAllocationPage>& currentPage)
{
	if (emptyPagePool.empty())
	{
		currentPage = std::make_shared<GraphicsBufferAllocationPage>(device, heapType, pageSize);

		usedPagePool.push_back(currentPage);
	}
	else
	{
		currentPage = emptyPagePool.front();

		emptyPagePool.pop_front();
	}
}
