#pragma once

#include "GraphicsHelper.h"

namespace Graphics
{
	struct BufferAllocation
	{
		uint8_t* cpuAddress;
		D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
		uint64_t gpuPageOffset;
		ID3D12Resource* bufferResource; //TODO: Check const
	};

	struct BufferAllocationPage
	{
	public:
		BufferAllocationPage(ID3D12Device* device, D3D12_HEAP_TYPE _heapType, D3D12_RESOURCE_FLAGS resourceFlags = D3D12_RESOURCE_FLAG_NONE, uint64_t _pageSize = 2 * _MB);
		~BufferAllocationPage();

		void Allocate(uint64_t _size, uint64_t alignment, BufferAllocation& allocation);
		bool HasSpace(uint64_t _size, uint64_t alignment) const noexcept;

	private:
		uint64_t pageSize;
		uint64_t offset;

		uint8_t* currentCPUAddress;
		D3D12_GPU_VIRTUAL_ADDRESS currentGPUAddress;

		D3D12_HEAP_TYPE heapType;

		ComPtr<ID3D12Resource> pageResource;
	};
}
