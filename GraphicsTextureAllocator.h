#pragma once

#include "GraphicsHelper.h"
#include "GraphicsTextureAllocationPage.h"

class GraphicsTextureAllocator
{
public:
	static GraphicsTextureAllocator& GetInstance();

	void Allocate(ID3D12Device* device, D3D12_RESOURCE_FLAGS resourceFlags, const TextureInfo& textureInfo, GraphicsTextureAllocation& allocation);
	void AllocateTemporaryUpload(ID3D12Device* device, D3D12_RESOURCE_FLAGS resourceFlags, const TextureInfo& textureInfo, GraphicsTextureAllocation& allocation);

	void ReleaseTemporaryBuffers();

private:
	GraphicsTextureAllocator() {};
	~GraphicsTextureAllocator() {};

	GraphicsTextureAllocator(const GraphicsTextureAllocator&) = delete;
	GraphicsTextureAllocator(GraphicsTextureAllocator&&) = delete;
	GraphicsTextureAllocator& operator=(const GraphicsTextureAllocator&) = delete;
	GraphicsTextureAllocator& operator=(GraphicsTextureAllocator&&) = delete;

	using TextureAllocationPagePool = std::deque<std::shared_ptr<GraphicsTextureAllocationPage>>;

	TextureAllocationPagePool pages;
	TextureAllocationPagePool tempUploadPages;
};
