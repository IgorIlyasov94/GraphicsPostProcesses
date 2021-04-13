#pragma once

#include "GraphicsHelper.h"
#include "TextureAllocationPage.h"

namespace Graphics
{
	class TextureAllocator
	{
	public:
		static TextureAllocator& GetInstance();

		void Allocate(ID3D12Device* device, D3D12_RESOURCE_FLAGS resourceFlags, const TextureInfo& textureInfo, TextureAllocation& allocation);
		void AllocateTemporaryUpload(ID3D12Device* device, D3D12_RESOURCE_FLAGS resourceFlags, const TextureInfo& textureInfo, TextureAllocation& allocation);

		void ReleaseTemporaryBuffers();

	private:
		TextureAllocator() {};
		~TextureAllocator() {};

		TextureAllocator(const TextureAllocator&) = delete;
		TextureAllocator(TextureAllocator&&) = delete;
		TextureAllocator& operator=(const TextureAllocator&) = delete;
		TextureAllocator& operator=(TextureAllocator&&) = delete;

		using TextureAllocationPagePool = std::deque<std::shared_ptr<TextureAllocationPage>>;

		TextureAllocationPagePool pages;
		TextureAllocationPagePool tempUploadPages;
	};
}
