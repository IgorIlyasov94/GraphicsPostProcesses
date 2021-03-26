#pragma once

#include "GraphicsDescriptorAllocation.h"

class GraphicsDescriptorAllocationPage
{
public:
	GraphicsDescriptorAllocationPage(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType, uint32_t _numDescriptors);
	~GraphicsDescriptorAllocationPage() {};

	void Allocate(uint32_t numDescriptors, GraphicsDescriptorAllocation& allocation);
	const uint32_t& GetNumFreeHandles() const;

private:

	uint32_t numDescriptors;
	uint32_t numFreeHandles;
	uint32_t descriptorIncrementSize;

	ComPtr<ID3D12DescriptorHeap> descriptorHeap;

	D3D12_CPU_DESCRIPTOR_HANDLE descriptorBase;

	const D3D12_DESCRIPTOR_HEAP_TYPE descriptorType;
};