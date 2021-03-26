#pragma once

#include "GraphicsHelper.h"
#include "GraphicsDescriptorAllocationPage.h"

class GraphicsDescriptorAllocator
{
public:
	static GraphicsDescriptorAllocator& GetInstance();

	void Allocate(ID3D12Device* device, const uint32_t numDescriptors, const D3D12_DESCRIPTOR_HEAP_TYPE descriptorType,
		GraphicsDescriptorAllocation& allocation);

private:
	GraphicsDescriptorAllocator() {};
	~GraphicsDescriptorAllocator() {};

	GraphicsDescriptorAllocator(const GraphicsDescriptorAllocator&) = delete;
	GraphicsDescriptorAllocator(GraphicsDescriptorAllocator&&) = delete;
	GraphicsDescriptorAllocator& operator=(const GraphicsDescriptorAllocator&) = delete;
	GraphicsDescriptorAllocator& operator=(GraphicsDescriptorAllocator&&) = delete;

	using DescriptorHeapPool = std::vector<std::shared_ptr<GraphicsDescriptorAllocationPage>>;

	void Allocate(ID3D12Device* device, const uint32_t numDescriptors, const D3D12_DESCRIPTOR_HEAP_TYPE descriptorType, DescriptorHeapPool& heapPool,
		std::set<size_t>& availableHeapIndices, GraphicsDescriptorAllocation& allocation);

	DescriptorHeapPool cbvSrvUavDescriptorHeapPages;
	DescriptorHeapPool samplerDescriptorHeapPages;
	DescriptorHeapPool rtvDescriptorHeapPages;
	DescriptorHeapPool dsvDescriptorHeapPages;

	std::set<size_t> availableCbvSrvUavHeapIndices;
	std::set<size_t> availableSamplerHeapIndices;
	std::set<size_t> availableRtvHeapIndices;
	std::set<size_t> availableDsvHeapIndices;

	uint32_t numDescriptorsPerHeap = 256;

	std::mutex descriptorAllocatorMutex;
};