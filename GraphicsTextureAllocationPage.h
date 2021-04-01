#pragma once

#include "GraphicsHelper.h"

struct GraphicsTextureAllocation
{
	uint8_t* cpuAddress;
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
	ID3D12Resource* textureResource;
};

class GraphicsTextureAllocationPage
{
public:
	GraphicsTextureAllocationPage(ID3D12Device* device, D3D12_HEAP_TYPE _heapType, D3D12_RESOURCE_FLAGS resourceFlags, const TextureInfo& textureInfo);
	~GraphicsTextureAllocationPage();

	void GetAllocation(GraphicsTextureAllocation& allocation);

private:
	uint8_t* cpuAddress;
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;

	D3D12_HEAP_TYPE heapType;

	ComPtr<ID3D12Resource> pageResource;
};
