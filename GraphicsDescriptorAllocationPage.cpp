#include "GraphicsDescriptorAllocationPage.h"

GraphicsDescriptorAllocationPage::GraphicsDescriptorAllocationPage(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType,
	uint32_t _numDescriptors)
	: numDescriptors(_numDescriptors), descriptorType(descriptorHeapType)
{
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Type = descriptorType;

	ThrowIfFailed(device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap)),
		"GraphicsDescriptorAllocationPage::GraphicsDescriptorAllocationPage: Descriptor Heap creating error");

	numFreeHandles = numDescriptors;
	descriptorIncrementSize =  device->GetDescriptorHandleIncrementSize(descriptorType);

	descriptorBase = descriptorHeap->GetCPUDescriptorHandleForHeapStart();

	//addNewBlock ?
}

void GraphicsDescriptorAllocationPage::Allocate(uint32_t numDescriptors, GraphicsDescriptorAllocation& allocation)
{

}

const uint32_t& GraphicsDescriptorAllocationPage::GetNumFreeHandles() const
{
	return numFreeHandles;
}