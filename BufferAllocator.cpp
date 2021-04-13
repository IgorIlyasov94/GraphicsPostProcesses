#include "BufferAllocator.h"

Graphics::BufferAllocator& Graphics::BufferAllocator::GetInstance()
{
	static BufferAllocator thisInstance;

	return thisInstance;
}

void Graphics::BufferAllocator::Allocate(ID3D12Device* device, size_t size, size_t alignment, D3D12_HEAP_TYPE heapType, BufferAllocation& allocation)
{
	if (size > pageSize)
		throw std::exception("BufferAllocator::Allocate: Bad allocation");

	if (heapType == D3D12_HEAP_TYPE_DEFAULT)
	{
		Allocate(device, size, alignment, heapType, emptyDefaultPages, usedDefaultPages, currentDefaultPage, allocation);
	}
	else if (heapType == D3D12_HEAP_TYPE_UPLOAD)
	{
		Allocate(device, size, alignment, heapType, emptyUploadPages, usedUploadPages, currentUploadPage, allocation);
	}
}

void Graphics::BufferAllocator::AllocateTemporaryUpload(ID3D12Device* device, size_t size, BufferAllocation& allocation)
{
	tempUploadPages.push_back(std::make_shared<BufferAllocationPage>(device, D3D12_HEAP_TYPE_UPLOAD, size));

	tempUploadPages[tempUploadPages.size() - 1]->Allocate(size, 1, allocation);
}

void Graphics::BufferAllocator::ReleaseTemporaryBuffers()
{
	tempUploadPages.clear();
}

void Graphics::BufferAllocator::Allocate(ID3D12Device* device, size_t size, size_t alignment, D3D12_HEAP_TYPE heapType, BufferAllocationPagePool& emptyPagePool,
	BufferAllocationPagePool& usedPagePool, std::shared_ptr<BufferAllocationPage>& currentPage, BufferAllocation& allocation)
{
	if (!currentPage || !currentPage->HasSpace(size, alignment))
		SetNewPageAsCurrent(device, heapType, emptyPagePool, usedPagePool, currentPage);

	currentPage->Allocate(size, alignment, allocation);
}

void Graphics::BufferAllocator::SetNewPageAsCurrent(ID3D12Device* device, D3D12_HEAP_TYPE heapType, BufferAllocationPagePool& emptyPagePool,
	BufferAllocationPagePool& usedPagePool, std::shared_ptr<BufferAllocationPage>& currentPage)
{
	if (emptyPagePool.empty())
	{
		currentPage = std::make_shared<BufferAllocationPage>(device, heapType, pageSize);

		usedPagePool.push_back(currentPage);
	}
	else
	{
		currentPage = emptyPagePool.front();

		emptyPagePool.pop_front();
	}
}
