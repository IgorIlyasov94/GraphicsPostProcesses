#pragma once

#include "GraphicsHelper.h"
#include "DescriptorAllocationPage.h"

namespace Graphics
{
	class DescriptorAllocator
	{
	public:
		static DescriptorAllocator& GetInstance();

		void Allocate(ID3D12Device* device, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE descriptorType, bool isShaderVisible,
			DescriptorAllocation& allocation);

	private:
		DescriptorAllocator() {};
		~DescriptorAllocator() {};

		DescriptorAllocator(const DescriptorAllocator&) = delete;
		DescriptorAllocator(DescriptorAllocator&&) = delete;
		DescriptorAllocator& operator=(const DescriptorAllocator&) = delete;
		DescriptorAllocator& operator=(DescriptorAllocator&&) = delete;

		using DescriptorHeapPool = std::deque<std::shared_ptr<DescriptorAllocationPage>>;

		void Allocate(ID3D12Device* device, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE descriptorType, bool isShaderVisible,
			DescriptorHeapPool& usedHeapPool, DescriptorHeapPool& emptyHeapPool, std::shared_ptr<DescriptorAllocationPage>& currentPage,
			DescriptorAllocation& allocation);

		void SetNewPageAsCurrent(ID3D12Device* device, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE descriptorType, bool isShaderVisible,
			DescriptorHeapPool& usedHeapPool, DescriptorHeapPool& emptyHeapPool, std::shared_ptr<DescriptorAllocationPage>& currentPage);

		DescriptorHeapPool usedCbvSrvUavDescriptorHeapPages;
		DescriptorHeapPool usedSamplerDescriptorHeapPages;
		DescriptorHeapPool usedRtvDescriptorHeapPages;
		DescriptorHeapPool usedDsvDescriptorHeapPages;
		DescriptorHeapPool usedShaderInvisibleRtvDescriptorHeapPages;
		DescriptorHeapPool usedShaderInvisibleDsvDescriptorHeapPages;

		DescriptorHeapPool emptyCbvSrvUavDescriptorHeapPages;
		DescriptorHeapPool emptySamplerDescriptorHeapPages;
		DescriptorHeapPool emptyRtvDescriptorHeapPages;
		DescriptorHeapPool emptyDsvDescriptorHeapPages;
		DescriptorHeapPool emptyShaderInvisibleRtvDescriptorHeapPages;
		DescriptorHeapPool emptyShaderInvisibleDsvDescriptorHeapPages;

		std::shared_ptr<DescriptorAllocationPage> currentCbvSrvUavDescriptorHeapPage;
		std::shared_ptr<DescriptorAllocationPage> currentSamplerDescriptorHeapPage;
		std::shared_ptr<DescriptorAllocationPage> currentRtvDescriptorHeapPage;
		std::shared_ptr<DescriptorAllocationPage> currentDsvDescriptorHeapPage;
		std::shared_ptr<DescriptorAllocationPage> currentShaderInvisibleRtvDescriptorHeapPage;
		std::shared_ptr<DescriptorAllocationPage> currentShaderInvisibleDsvDescriptorHeapPage;

		uint32_t numDescriptorsPerHeap = 256;
	};
}
