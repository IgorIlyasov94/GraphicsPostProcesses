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
		Allocate(device, size, alignment, heapType, false, false, emptyDefaultPages, usedDefaultPages, currentDefaultPage, allocation);
	}
	else if (heapType == D3D12_HEAP_TYPE_UPLOAD)
	{
		Allocate(device, size, alignment, heapType, false, false, emptyUploadPages, usedUploadPages, currentUploadPage, allocation);
	}
}

void Graphics::BufferAllocator::AllocateCustomBuffer(ID3D12Device* device, size_t size, size_t alignment, BufferAllocation& allocation)
{
	if (size > pageSize)
		throw std::exception("BufferAllocator::AllocateCustomBuffer: Bad allocation");

	Allocate(device, size, alignment, D3D12_HEAP_TYPE_DEFAULT, false, true, emptyCustomPages, usedCustomPages, currentCustomPage, allocation);
}

void Graphics::BufferAllocator::AllocateUnorderedAccess(ID3D12Device* device, size_t size, size_t alignment, BufferAllocation& allocation)
{
	if (size > pageSize)
		throw std::exception("BufferAllocator::AllocateUnorderedAccess: Bad allocation");

	Allocate(device, size, alignment, D3D12_HEAP_TYPE_DEFAULT, true, true, emptyUnorderedPages, usedUnorderedPages, currentUnorderedPage, allocation);
}

void Graphics::BufferAllocator::AllocateTemporary(ID3D12Device* device, size_t size, D3D12_HEAP_TYPE heapType, BufferAllocation& allocation)
{
	tempUploadPages.push_back(std::shared_ptr<BufferAllocationPage>(new BufferAllocationPage(device, heapType, D3D12_RESOURCE_FLAG_NONE, size)));

	tempUploadPages.back()->Allocate(size, 1, allocation);
}

void Graphics::BufferAllocator::ReleaseTemporaryBuffers()
{
	tempUploadPages.clear();
}

void Graphics::BufferAllocator::Allocate(ID3D12Device* device, size_t size, size_t alignment, D3D12_HEAP_TYPE heapType, bool unorderedAccess , bool isUniqueBuffer,
	BufferAllocationPagePool& emptyPagePool, BufferAllocationPagePool& usedPagePool, std::shared_ptr<BufferAllocationPage>& currentPage, BufferAllocation& allocation)
{
	if (!currentPage || !currentPage->HasSpace(size, alignment) || isUniqueBuffer)
		SetNewPageAsCurrent(device, heapType, unorderedAccess, emptyPagePool, usedPagePool, currentPage);

	currentPage->Allocate(size, alignment, allocation);
}

void Graphics::BufferAllocator::SetNewPageAsCurrent(ID3D12Device* device, D3D12_HEAP_TYPE heapType, bool unorderedAccess, BufferAllocationPagePool& emptyPagePool,
	BufferAllocationPagePool& usedPagePool, std::shared_ptr<BufferAllocationPage>& currentPage)
{
	if (emptyPagePool.empty())
	{
		currentPage = std::shared_ptr<BufferAllocationPage>(new BufferAllocationPage(device, heapType,
			(unorderedAccess) ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE, pageSize));

		usedPagePool.push_back(currentPage);
	}
	else
	{
		currentPage = emptyPagePool.front();

		emptyPagePool.pop_front();
	}
}
