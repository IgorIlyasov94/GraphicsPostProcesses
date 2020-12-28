#pragma once

#include "GraphicsHelper.h"

struct GraphicsBufferAllocation
{
	uint8_t* cpuAddress;
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
};

struct GraphicsBufferAllocationPage
{
public:
	GraphicsBufferAllocationPage(ID3D12Device* device, size_t _pageSize);
	~GraphicsBufferAllocationPage();

	void Allocate(size_t _size, size_t alignment, GraphicsBufferAllocation& allocation);

	bool HasSpace(size_t _size, size_t alignment);

private:
	size_t currentPageSize;
	size_t offset;

	uint8_t* currentCPUAddress;
	D3D12_GPU_VIRTUAL_ADDRESS currentGPUAddress;

	ComPtr<ID3D12Resource> pageResource;
};