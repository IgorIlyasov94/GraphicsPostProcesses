#include "TextureAllocator.h"

Graphics::TextureAllocator& Graphics::TextureAllocator::GetInstance()
{
    static TextureAllocator thisInstance;

    return thisInstance;
}

void Graphics::TextureAllocator::Allocate(ID3D12Device* device, D3D12_RESOURCE_FLAGS resourceFlags, const TextureInfo& textureInfo,
    TextureAllocation& allocation)
{
    pages.push_back(std::make_shared<TextureAllocationPage>(device, D3D12_HEAP_TYPE_DEFAULT, resourceFlags, textureInfo));

    pages[pages.size() - 1]->GetAllocation(allocation);
}

void Graphics::TextureAllocator::AllocateTemporaryUpload(ID3D12Device* device, D3D12_RESOURCE_FLAGS resourceFlags, const TextureInfo& textureInfo,
    TextureAllocation& allocation)
{
    tempUploadPages.push_back(std::make_shared<TextureAllocationPage>(device, D3D12_HEAP_TYPE_UPLOAD, resourceFlags, textureInfo));

    tempUploadPages[tempUploadPages.size() - 1]->GetAllocation(allocation);
}

void Graphics::TextureAllocator::ReleaseTemporaryBuffers()
{
    tempUploadPages.clear();
}
