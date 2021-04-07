#pragma once

#include "GraphicsHelper.h"

struct GraphicsDescriptorAllocation
{
	uint32_t descriptorStartIndex;
	UINT descriptorIncrementSize;
	D3D12_CPU_DESCRIPTOR_HANDLE descriptorBase;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorBase;
	ID3D12DescriptorHeap* descriptorHeap;
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
	UINT64 gpuDescriptorBaseOffset;
	D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType;
};
