#pragma once

#include "GraphicsHelper.h"
#include "GraphicsDescriptorAllocationPage.h"

class GraphicsDescriptorAllocator
{
public:
	static GraphicsDescriptorAllocator& GetInstance();

	void Allocate(ID3D12Device* device, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE descriptorType, GraphicsDescriptorAllocation& allocation);

private:
	GraphicsDescriptorAllocator() {};
	~GraphicsDescriptorAllocator() {};

	GraphicsDescriptorAllocator(const GraphicsDescriptorAllocator&) = delete;
	GraphicsDescriptorAllocator(GraphicsDescriptorAllocator&&) = delete;
	GraphicsDescriptorAllocator& operator=(const GraphicsDescriptorAllocator&) = delete;
	GraphicsDescriptorAllocator& operator=(GraphicsDescriptorAllocator&&) = delete;

	using DescriptorHeapPool = std::deque<std::shared_ptr<GraphicsDescriptorAllocationPage>>;

	void Allocate(ID3D12Device* device, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE descriptorType, DescriptorHeapPool& usedHeapPool,
		DescriptorHeapPool& emptyHeapPool, std::shared_ptr<GraphicsDescriptorAllocationPage>& currentPage, GraphicsDescriptorAllocation& allocation);

	void SetNewPageAsCurrent(ID3D12Device* device, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE descriptorType, DescriptorHeapPool& usedHeapPool,
		DescriptorHeapPool& emptyHeapPool, std::shared_ptr<GraphicsDescriptorAllocationPage>& currentPage);

	DescriptorHeapPool usedCbvSrvUavDescriptorHeapPages;
	DescriptorHeapPool usedSamplerDescriptorHeapPages;
	DescriptorHeapPool usedRtvDescriptorHeapPages;
	DescriptorHeapPool usedDsvDescriptorHeapPages;

	DescriptorHeapPool emptyCbvSrvUavDescriptorHeapPages;
	DescriptorHeapPool emptySamplerDescriptorHeapPages;
	DescriptorHeapPool emptyRtvDescriptorHeapPages;
	DescriptorHeapPool emptyDsvDescriptorHeapPages;

	std::shared_ptr<GraphicsDescriptorAllocationPage> currentCbvSrvUavDescriptorHeapPage;
	std::shared_ptr<GraphicsDescriptorAllocationPage> currentSamplerDescriptorHeapPage;
	std::shared_ptr<GraphicsDescriptorAllocationPage> currentRtvUavDescriptorHeapPage;
	std::shared_ptr<GraphicsDescriptorAllocationPage> currentDsvDescriptorHeapPage;

	uint32_t numDescriptorsPerHeap = 256;
};
