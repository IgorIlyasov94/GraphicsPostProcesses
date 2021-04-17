#include "DescriptorAllocator.h"

Graphics::DescriptorAllocator& Graphics::DescriptorAllocator::GetInstance()
{
    static DescriptorAllocator thisInstance;

    return thisInstance;
}

void Graphics::DescriptorAllocator::Allocate(ID3D12Device* device, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE descriptorType, 
    bool isShaderVisible, DescriptorAllocation& allocation)
{
    if (descriptorType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
        Allocate(device, numDescriptors, descriptorType, true, usedCbvSrvUavDescriptorHeapPages, emptyCbvSrvUavDescriptorHeapPages,
            currentCbvSrvUavDescriptorHeapPage, allocation);
    else if (descriptorType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
        Allocate(device, numDescriptors, descriptorType, true, usedSamplerDescriptorHeapPages, emptySamplerDescriptorHeapPages,
            currentSamplerDescriptorHeapPage, allocation);
    else if (descriptorType == D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
    {
        if (isShaderVisible)
            Allocate(device, numDescriptors, descriptorType, true, usedRtvDescriptorHeapPages, emptyRtvDescriptorHeapPages,
                currentRtvDescriptorHeapPage, allocation);
        else
            Allocate(device, numDescriptors, descriptorType, false, usedShaderInvisibleRtvDescriptorHeapPages, emptyShaderInvisibleRtvDescriptorHeapPages,
                currentShaderInvisibleRtvDescriptorHeapPage, allocation);
    }
    else if (descriptorType == D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
    {
        if (isShaderVisible)
            Allocate(device, numDescriptors, descriptorType, true, usedDsvDescriptorHeapPages, emptyDsvDescriptorHeapPages,
                currentDsvDescriptorHeapPage, allocation);
        else
            Allocate(device, numDescriptors, descriptorType, false, usedShaderInvisibleDsvDescriptorHeapPages, emptyShaderInvisibleDsvDescriptorHeapPages,
                currentShaderInvisibleDsvDescriptorHeapPage, allocation);
    }
}

void Graphics::DescriptorAllocator::Allocate(ID3D12Device* device, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE descriptorType,
    bool isShaderVisible, DescriptorHeapPool& usedHeapPool, DescriptorHeapPool& emptyHeapPool, std::shared_ptr<DescriptorAllocationPage>& currentPage,
    DescriptorAllocation& allocation)
{
    if (currentPage.get() == nullptr || !currentPage->HasSpace(numDescriptors))
        SetNewPageAsCurrent(device, numDescriptors, descriptorType, isShaderVisible, usedHeapPool, emptyHeapPool, currentPage);

    currentPage->Allocate(numDescriptors, allocation);
}

void Graphics::DescriptorAllocator::SetNewPageAsCurrent(ID3D12Device* device, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE descriptorType,
    bool isShaderVisible, DescriptorHeapPool& usedHeapPool, DescriptorHeapPool& emptyHeapPool, std::shared_ptr<DescriptorAllocationPage>& currentPage)
{
    if (emptyHeapPool.empty())
    {
        currentPage = std::make_shared<DescriptorAllocationPage>(device, descriptorType, numDescriptorsPerHeap, isShaderVisible);

        usedHeapPool.push_back(currentPage);
    }
    else
    {
        currentPage = emptyHeapPool.front();

        emptyHeapPool.pop_front();
    }
}
