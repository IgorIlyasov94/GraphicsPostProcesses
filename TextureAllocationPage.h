#pragma once

#include "GraphicsHelper.h"

namespace Graphics
{
	struct TextureAllocation
	{
		uint8_t* cpuAddress;
		ID3D12Resource* textureResource;
	};

	class TextureAllocationPage
	{
	public:
		TextureAllocationPage(ID3D12Device* device, D3D12_HEAP_TYPE _heapType, D3D12_RESOURCE_FLAGS resourceFlags, const TextureInfo& textureInfo);
		~TextureAllocationPage();

		void GetAllocation(TextureAllocation& allocation);

	private:
		uint8_t* cpuAddress;

		D3D12_HEAP_TYPE heapType;

		ComPtr<ID3D12Resource> pageResource;
	};
}
