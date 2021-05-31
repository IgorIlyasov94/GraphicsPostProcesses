#pragma once

#include "GraphicsHelper.h"
#include "DescriptorAllocationPage.h"

namespace Graphics
{
	class DescriptorAllocator
	{
	public:
		static DescriptorAllocator& GetInstance();

		void Allocate(ID3D12Device* device, bool isUAVShaderNonVisible, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE descriptorType, DescriptorAllocation& allocation);

	private:
		DescriptorAllocator() {};
		~DescriptorAllocator() {};

		DescriptorAllocator(const DescriptorAllocator&) = delete;
		DescriptorAllocator(DescriptorAllocator&&) = delete;
		DescriptorAllocator& operator=(const DescriptorAllocator&) = delete;
		DescriptorAllocator& operator=(DescriptorAllocator&&) = delete;

		using DescriptorHeapPool = std::deque<std::shared_ptr<DescriptorAllocationPage>>;

		void Allocate(ID3D12Device* device, bool isUAVShaderNonVisible, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE descriptorType, DescriptorHeapPool& usedHeapPool,
			DescriptorHeapPool& emptyHeapPool, std::shared_ptr<DescriptorAllocationPage>& currentPage, DescriptorAllocation& allocation);

		void SetNewPageAsCurrent(ID3D12Device* device, bool isUAVShaderNonVisible, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE descriptorType,
			DescriptorHeapPool& usedHeapPool, DescriptorHeapPool& emptyHeapPool, std::shared_ptr<DescriptorAllocationPage>& currentPage);

		DescriptorHeapPool usedCbvSrvUavDescriptorHeapPages;
		DescriptorHeapPool usedSamplerDescriptorHeapPages;
		DescriptorHeapPool usedRtvDescriptorHeapPages;
		DescriptorHeapPool usedDsvDescriptorHeapPages;
		DescriptorHeapPool usedUavShaderNonVisibleDescriptorHeapPages;

		DescriptorHeapPool emptyCbvSrvUavDescriptorHeapPages;
		DescriptorHeapPool emptySamplerDescriptorHeapPages;
		DescriptorHeapPool emptyRtvDescriptorHeapPages;
		DescriptorHeapPool emptyDsvDescriptorHeapPages;
		DescriptorHeapPool emptyUavShaderNonVisibleDescriptorHeapPages;

		std::shared_ptr<DescriptorAllocationPage> currentCbvSrvUavDescriptorHeapPage;
		std::shared_ptr<DescriptorAllocationPage> currentSamplerDescriptorHeapPage;
		std::shared_ptr<DescriptorAllocationPage> currentRtvDescriptorHeapPage;
		std::shared_ptr<DescriptorAllocationPage> currentDsvDescriptorHeapPage;
		std::shared_ptr<DescriptorAllocationPage> currentUavShaderNonVisibleDescriptorHeapPage;

		uint32_t numDescriptorsPerHeap = 256;
	};
}
