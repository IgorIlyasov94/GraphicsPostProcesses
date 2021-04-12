#include "GraphicsDescriptorAllocationPage.h"

GraphicsDescriptorAllocationPage::GraphicsDescriptorAllocationPage(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_TYPE _descriptorHeapType,
	uint32_t _numDescriptors)
	: numDescriptors(_numDescriptors), descriptorHeapType(_descriptorHeapType), descriptorBaseOffset(0u), gpuDescriptorBaseOffset(0u)
{
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Type = descriptorHeapType;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	
	ThrowIfFailed(device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap)),
		"GraphicsDescriptorAllocationPage::GraphicsDescriptorAllocationPage: Descriptor Heap creating error");

	numFreeHandles = numDescriptors;
	descriptorIncrementSize =  device->GetDescriptorHandleIncrementSize(descriptorHeapType);

	descriptorBaseOffset = descriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr;
	gpuDescriptorBaseOffset = descriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr;
}

void GraphicsDescriptorAllocationPage::Allocate(uint32_t _numDescriptors, GraphicsDescriptorAllocation& allocation)
{
	if (numFreeHandles < _numDescriptors)
		throw std::exception("GraphicsDescriptorAllocationPage::Allocate: Bad allocation");

	allocation.descriptorBase.ptr = descriptorBaseOffset;
	allocation.gpuDescriptorBase.ptr = gpuDescriptorBaseOffset;
	allocation.descriptorIncrementSize = descriptorIncrementSize;
	allocation.descriptorStartIndex = numDescriptors - numFreeHandles;
	allocation.descriptorHeap = descriptorHeap.Get();

	descriptorBaseOffset += static_cast<SIZE_T>(static_cast<INT64>(_numDescriptors) * static_cast<INT64>(descriptorIncrementSize));
	gpuDescriptorBaseOffset += static_cast<UINT64>(static_cast<INT64>(_numDescriptors) * static_cast<INT64>(descriptorIncrementSize));
	numFreeHandles -= _numDescriptors;
}

bool GraphicsDescriptorAllocationPage::HasSpace(uint32_t _numDescriptors) const noexcept
{
	return numFreeHandles > _numDescriptors;
}
