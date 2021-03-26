#include "GraphicsBufferAllocator.h"

GraphicsBufferAllocator& GraphicsBufferAllocator::GetInstance()
{
	static GraphicsBufferAllocator thisInstance;

	return thisInstance;
}

void GraphicsBufferAllocator::AllocateDefault(ID3D12Device* device, size_t size, size_t alignment, GraphicsBufferAllocation& allocation)
{
	if (size > pageSize)
		throw std::bad_alloc();

	if (!currentDefaultPage || !currentDefaultPage->HasSpace(size, alignment))
		SetNewDefaultPageAsCurrent(device, currentDefaultPage);

	currentDefaultPage->Allocate(size, alignment, allocation);
}

void GraphicsBufferAllocator::AllocateUpload(ID3D12Device* device, size_t size, size_t alignment, GraphicsBufferAllocation& allocation)
{
	if (size > pageSize)
		throw std::bad_alloc();

	if (!currentUploadPage || !currentUploadPage->HasSpace(size, alignment))
		SetNewUploadPageAsCurrent(device, currentUploadPage);

	currentUploadPage->Allocate(size, alignment, allocation);
}

void GraphicsBufferAllocator::SetNewDefaultPageAsCurrent(ID3D12Device* device, std::shared_ptr<DefaultBufferAllocation>& oldCurrentPage)
{
	if (emptyDefaultPages.empty())
	{
		GraphicsBufferAllocationPage<D3D12_HEAP_TYPE_DEFAULT> newPage(device, pageSize);

		oldCurrentPage = std::make_shared<GraphicsBufferAllocationPage<D3D12_HEAP_TYPE_DEFAULT>>(device, pageSize);

		usedDefaultPages.push_back(oldCurrentPage);
	}
	else
	{
		oldCurrentPage = emptyDefaultPages.front();

		emptyDefaultPages.pop_front();
	}
}

void GraphicsBufferAllocator::SetNewUploadPageAsCurrent(ID3D12Device* device, std::shared_ptr<UploadBufferAllocation>& oldCurrentPage)
{
	if (emptyUploadPages.empty())
	{
		GraphicsBufferAllocationPage<D3D12_HEAP_TYPE_UPLOAD> newPage(device, pageSize);

		oldCurrentPage = std::make_shared<GraphicsBufferAllocationPage<D3D12_HEAP_TYPE_UPLOAD>>(device, pageSize);

		usedUploadPages.push_back(oldCurrentPage);
	}
	else
	{
		oldCurrentPage = emptyUploadPages.front();

		emptyUploadPages.pop_front();
	}
}
