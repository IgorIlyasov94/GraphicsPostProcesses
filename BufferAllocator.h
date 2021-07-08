#pragma once

#include "GraphicsHelper.h"
#include "BufferAllocationPage.h"

namespace Graphics
{
	class BufferAllocator
	{
	public:
		static BufferAllocator& GetInstance();

		void Allocate(ID3D12Device* device, size_t size, size_t alignment, D3D12_HEAP_TYPE heapType, BufferAllocation& allocation);
		void AllocateCustomBuffer(ID3D12Device* device, size_t size, size_t alignment, BufferAllocation& allocation);
		void AllocateUnorderedAccess(ID3D12Device* device, size_t size, size_t alignment, BufferAllocation& allocation);
		void AllocateTemporary(ID3D12Device* device, size_t size, D3D12_HEAP_TYPE heapType, BufferAllocation& allocation);

		void ReleaseTemporaryBuffers();

	private:
		BufferAllocator() {};
		~BufferAllocator() {};

		BufferAllocator(const BufferAllocator&) = delete;
		BufferAllocator(BufferAllocator&&) = delete;
		BufferAllocator& operator=(const BufferAllocator&) = delete;
		BufferAllocator& operator=(BufferAllocator&&) = delete;

		using BufferAllocationPagePool = std::deque<std::shared_ptr<BufferAllocationPage>>;

		void Allocate(ID3D12Device* device, size_t size, size_t alignment, D3D12_HEAP_TYPE heapType, bool unorderedAccess, bool isUniqueBuffer, BufferAllocationPagePool& emptyPagePool,
			BufferAllocationPagePool& usedPagePool, std::shared_ptr<BufferAllocationPage>& currentPage, BufferAllocation& allocation);

		void SetNewPageAsCurrent(ID3D12Device* device, D3D12_HEAP_TYPE heapType, bool unorderedAccess, BufferAllocationPagePool& emptyPagePool,
			BufferAllocationPagePool& usedPagePool, std::shared_ptr<BufferAllocationPage>& currentPage);

		BufferAllocationPagePool usedDefaultPages;
		BufferAllocationPagePool usedUploadPages;
		BufferAllocationPagePool usedCustomPages;
		BufferAllocationPagePool usedUnorderedPages;

		BufferAllocationPagePool emptyDefaultPages;
		BufferAllocationPagePool emptyUploadPages;
		BufferAllocationPagePool emptyCustomPages;
		BufferAllocationPagePool emptyUnorderedPages;

		std::shared_ptr<BufferAllocationPage> currentDefaultPage;
		std::shared_ptr<BufferAllocationPage> currentUploadPage;
		std::shared_ptr<BufferAllocationPage> currentCustomPage;
		std::shared_ptr<BufferAllocationPage> currentUnorderedPage;

		BufferAllocationPagePool tempUploadPages;

		const size_t pageSize = 2 * _MB;
	};
}
