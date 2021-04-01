#pragma once

#include "GraphicsHelper.h"

struct GraphicsDescriptorAllocation
{
	uint32_t numDescriptors;
	UINT descriptorIncrementSize;
	D3D12_CPU_DESCRIPTOR_HANDLE descriptorBase;
};

class GraphicsDescriptorAllocationPage
{
public:
	GraphicsDescriptorAllocationPage(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_TYPE _descriptorHeapType, uint32_t _numDescriptors);
	~GraphicsDescriptorAllocationPage() {};

	void Allocate(uint32_t _numDescriptors, GraphicsDescriptorAllocation& allocation);
	bool HasSpace(uint32_t _numDescriptors) const noexcept;

private:
	uint32_t numDescriptors;
	uint32_t numFreeHandles;
	UINT descriptorIncrementSize;

	ComPtr<ID3D12DescriptorHeap> descriptorHeap;

	SIZE_T descriptorBaseOffset;
	D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType;
};
