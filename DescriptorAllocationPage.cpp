#include "DescriptorAllocationPage.h"

Graphics::DescriptorAllocationPage::DescriptorAllocationPage(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE _descriptorHeapType,
	bool isUAVShaderNonVisible, uint32_t _numDescriptors)
	: numDescriptors(_numDescriptors), descriptorHeapType(_descriptorHeapType), descriptorBaseOffset(0u), gpuDescriptorBaseOffset(0u)
{
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Type = descriptorHeapType;

	if (descriptorHeapType == D3D12_DESCRIPTOR_HEAP_TYPE_RTV || descriptorHeapType == D3D12_DESCRIPTOR_HEAP_TYPE_DSV ||
		descriptorHeapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV && isUAVShaderNonVisible)
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	else
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	
	ThrowIfFailed(device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap)),
		"DescriptorAllocationPage::DescriptorAllocationPage: Descriptor Heap creating error");

	numFreeHandles = numDescriptors;
	descriptorIncrementSize =  device->GetDescriptorHandleIncrementSize(descriptorHeapType);

	descriptorBaseOffset = descriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr;
	gpuDescriptorBaseOffset = descriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr;
}

void Graphics::DescriptorAllocationPage::Allocate(uint32_t _numDescriptors, DescriptorAllocation& allocation)
{
	if (numFreeHandles < _numDescriptors)
		throw std::exception("DescriptorAllocationPage::Allocate: Bad allocation");

	allocation.descriptorBase.ptr = descriptorBaseOffset;
	allocation.gpuDescriptorBase.ptr = gpuDescriptorBaseOffset;
	allocation.descriptorIncrementSize = descriptorIncrementSize;
	allocation.descriptorHeap = descriptorHeap.Get();

	descriptorBaseOffset += static_cast<SIZE_T>(static_cast<INT64>(_numDescriptors) * static_cast<INT64>(descriptorIncrementSize));
	gpuDescriptorBaseOffset += static_cast<UINT64>(static_cast<INT64>(_numDescriptors) * static_cast<INT64>(descriptorIncrementSize));
	numFreeHandles -= _numDescriptors;
}

bool Graphics::DescriptorAllocationPage::HasSpace(uint32_t _numDescriptors) const noexcept
{
	return numFreeHandles > _numDescriptors;
}
