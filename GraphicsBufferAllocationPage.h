#pragma once

#include "GraphicsHelper.h"

struct GraphicsBufferAllocation
{
	uint8_t* cpuAddress;
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
	ID3D12Resource* bufferResource; //TODO: Check const
};

template<D3D12_HEAP_TYPE HeapType>
struct GraphicsBufferAllocationPage
{
public:
	GraphicsBufferAllocationPage(ID3D12Device* device, size_t _pageSize = 2 * _MB)
		: pageSize(_pageSize), offset(0), currentCPUAddress(nullptr), currentGPUAddress(D3D12_GPU_VIRTUAL_ADDRESS(0))
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

		D3D12_RANGE range = { 0, 0 };

		if (HeapType == D3D12_HEAP_TYPE_UPLOAD)
			ThrowIfFailed(pageResource->Map(0, &range, reinterpret_cast<void**>(&currentCPUAddress)),
				"GraphicsBufferAllocationPage<HeapType>::GraphicsBufferAllocationPage: Resource mapping error");

		currentGPUAddress = pageResource->GetGPUVirtualAddress();
	}

	~GraphicsBufferAllocationPage()
	{
		if (HeapType == D3D12_HEAP_TYPE_UPLOAD)
			pageResource->Unmap(0, nullptr);

		currentCPUAddress = nullptr;
		currentGPUAddress = D3D12_GPU_VIRTUAL_ADDRESS(0);

		offset = 0;
	}

	void Allocate(size_t _size, size_t alignment, GraphicsBufferAllocation& allocation)
	{
		if (!HasSpace(_size, alignment))
			throw std::exception("GraphicsBufferAllocationPage<HeapType>::Allocate: Bad allocation");

		auto alignedSize = AlignSize(_size, alignment);
		offset = AlignSize(offset, alignment);

		allocation.cpuAddress = currentCPUAddress + offset;
		allocation.gpuAddress = currentGPUAddress + offset;
		allocation.bufferResource = pageResource.Get();

		offset += alignedSize;
	}

	bool HasSpace(size_t _size, size_t alignment)
	{
		auto alignedSize = AlignSize(_size, alignment);
		auto alignedOffset = AlignSize(offset, alignment);

		return alignedSize + alignedOffset <= pageSize;
	}

private:
	size_t pageSize;
	size_t offset;

	uint8_t* currentCPUAddress;
	D3D12_GPU_VIRTUAL_ADDRESS currentGPUAddress;

	ComPtr<ID3D12Resource> pageResource;
};
