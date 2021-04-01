#include "GraphicsTextureAllocator.h"

GraphicsTextureAllocator& GraphicsTextureAllocator::GetInstance()
{
    static GraphicsTextureAllocator thisInstance;

    return thisInstance;
}

void GraphicsTextureAllocator::Allocate(ID3D12Device* device, D3D12_RESOURCE_FLAGS resourceFlags, const TextureInfo& textureInfo,
    GraphicsTextureAllocation& allocation)
{
    pages.push_back(std::make_shared<GraphicsTextureAllocationPage>(device, D3D12_HEAP_TYPE_DEFAULT, resourceFlags, textureInfo));

    pages[pages.size() - 1]->GetAllocation(allocation);
}

void GraphicsTextureAllocator::AllocateTemporaryUpload(ID3D12Device* device, D3D12_RESOURCE_FLAGS resourceFlags, const TextureInfo& textureInfo,
    GraphicsTextureAllocation& allocation)
{
    tempUploadPages.push_back(std::make_shared<GraphicsTextureAllocationPage>(device, D3D12_HEAP_TYPE_UPLOAD, resourceFlags, textureInfo));

    tempUploadPages[tempUploadPages.size() - 1]->GetAllocation(allocation);
}

void GraphicsTextureAllocator::ReleaseTemporaryBuffers()
{
    tempUploadPages.clear();
}
