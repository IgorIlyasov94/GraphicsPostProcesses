#include "GraphicsBufferAllocationPage.h"

GraphicsBufferAllocationPage::GraphicsBufferAllocationPage(ID3D12Device* device, size_t _pageSize)
	: currentPageSize(_pageSize), offset(0), currentCPUAddress(nullptr), currentGPUAddress(D3D12_GPU_VIRTUAL_ADDRESS(0))
{
	D3D12_HEAP_PROPERTIES heapProperties;
	SetupHeapProperties(heapProperties, D3D12_HEAP_TYPE_UPLOAD);

	D3D12_RESOURCE_DESC resourceDesc;
	SetupResourceBufferDesc(resourceDesc, _pageSize);

	ThrowIfFailed(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&pageResource)));

	pageResource->Map(0, nullptr, reinterpret_cast<void**>(&currentCPUAddress));
	currentGPUAddress = pageResource->GetGPUVirtualAddress();
}

GraphicsBufferAllocationPage::~GraphicsBufferAllocationPage()
{
	pageResource->Unmap(0, nullptr);

	currentCPUAddress = nullptr;
	currentGPUAddress = D3D12_GPU_VIRTUAL_ADDRESS(0);

	offset = 0;
}

void GraphicsBufferAllocationPage::Allocate(size_t _size, size_t alignment, GraphicsBufferAllocation& allocation)
{
	if (!HasSpace(_size, alignment))
		throw std::bad_alloc();

	auto alignedSize = AlignSize(_size, alignment);
	offset = AlignSize(offset, alignment);

	allocation.cpuAddress = currentCPUAddress + offset;
	allocation.gpuAddress = currentGPUAddress + offset;

	offset += alignedSize;
}

bool GraphicsBufferAllocationPage::HasSpace(size_t _size, size_t alignment)
{
	auto alignedSize = AlignSize(_size, alignment);
	auto alignedOffset = AlignSize(offset, alignment);

	return alignedSize + alignedOffset <= _size;
}
