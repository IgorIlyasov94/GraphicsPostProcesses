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
		void AllocateTemporaryUpload(ID3D12Device* device, size_t size, BufferAllocation& allocation);

		void ReleaseTemporaryBuffers();

	private:
		BufferAllocator() {};
		~BufferAllocator() {};

		BufferAllocator(const BufferAllocator&) = delete;
		BufferAllocator(BufferAllocator&&) = delete;
		BufferAllocator& operator=(const BufferAllocator&) = delete;
		BufferAllocator& operator=(BufferAllocator&&) = delete;

		using BufferAllocationPagePool = std::deque<std::shared_ptr<BufferAllocationPage>>;

		void Allocate(ID3D12Device* device, size_t size, size_t alignment, D3D12_HEAP_TYPE heapType, BufferAllocationPagePool& emptyPagePool,
			BufferAllocationPagePool& usedPagePool, std::shared_ptr<BufferAllocationPage>& currentPage, BufferAllocation& allocation);

		void SetNewPageAsCurrent(ID3D12Device* device, D3D12_HEAP_TYPE heapType, BufferAllocationPagePool& emptyPagePool,
			BufferAllocationPagePool& usedPagePool, std::shared_ptr<BufferAllocationPage>& currentPage);

		BufferAllocationPagePool usedDefaultPages;
		BufferAllocationPagePool usedUploadPages;

		BufferAllocationPagePool emptyDefaultPages;
		BufferAllocationPagePool emptyUploadPages;

		std::shared_ptr<BufferAllocationPage> currentDefaultPage;
		std::shared_ptr<BufferAllocationPage> currentUploadPage;

		BufferAllocationPagePool tempUploadPages;

		const size_t pageSize = 2 * _MB;
	};
}
