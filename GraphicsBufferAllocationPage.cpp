#include "GraphicsBufferAllocationPage.h"
/*
template<D3D12_HEAP_TYPE HeapType>
GraphicsBufferAllocationPage<HeapType>::GraphicsBufferAllocationPage(ID3D12Device* device, size_t _pageSize)
	: currentPageSize(_pageSize), offset(0), currentCPUAddress(nullptr), currentGPUAddress(D3D12_GPU_VIRTUAL_ADDRESS(0))
{
	D3D12_HEAP_PROPERTIES heapProperties;
	SetupHeapProperties(heapProperties, HeapType);

	D3D12_RESOURCE_DESC resourceDesc;
	SetupResourceBufferDesc(resourceDesc, _pageSize);

	D3D12_RESOURCE_STATES resourceState;

	if (HeapType == D3D12_HEAP_TYPE_UPLOAD)
		resourceState = D3D12_RESOURCE_STATE_GENERIC_READ;
	else
		resourceState = D3D12_RESOURCE_STATE_COPY_DEST;

	ThrowIfFailed(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, resourceState, nullptr,
		IID_PPV_ARGS(&pageResource)), "GraphicsBufferAllocationPage<HeapType>::GraphicsBufferAllocationPage: Resource creating error");

	ThrowIfFailed(pageResource->Map(0, nullptr, reinterpret_cast<void**>(&currentCPUAddress)), 
		"GraphicsBufferAllocationPage<HeapType>::GraphicsBufferAllocationPage: Resource mapping error");
	currentGPUAddress = pageResource->GetGPUVirtualAddress();
}

template<D3D12_HEAP_TYPE HeapType>
GraphicsBufferAllocationPage<HeapType>::~GraphicsBufferAllocationPage()
{
	pageResource->Unmap(0, nullptr);

	currentCPUAddress = nullptr;
	currentGPUAddress = D3D12_GPU_VIRTUAL_ADDRESS(0);

	offset = 0;
}

template<D3D12_HEAP_TYPE HeapType>
void GraphicsBufferAllocationPage<HeapType>::Allocate(size_t _size, size_t alignment, GraphicsBufferAllocation& allocation)
{
	if (!HasSpace(_size, alignment))
		throw std::bad_alloc();

	auto alignedSize = AlignSize(_size, alignment);
	offset = AlignSize(offset, alignment);

	allocation.cpuAddress = currentCPUAddress + offset;
	allocation.gpuAddress = currentGPUAddress + offset;
	allocation.bufferResource = pageResource.Get();

	offset += alignedSize;
}

template<D3D12_HEAP_TYPE HeapType>
bool GraphicsBufferAllocationPage<HeapType>::HasSpace(size_t _size, size_t alignment)
{
	auto alignedSize = AlignSize(_size, alignment);
	auto alignedOffset = AlignSize(offset, alignment);

	return alignedSize + alignedOffset <= _size;
}
*/