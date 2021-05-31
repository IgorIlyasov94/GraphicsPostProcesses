#include "DescriptorAllocator.h"

Graphics::DescriptorAllocator& Graphics::DescriptorAllocator::GetInstance()
{
    static DescriptorAllocator thisInstance;

    return thisInstance;
}

void Graphics::DescriptorAllocator::Allocate(ID3D12Device* device, bool isUAVShaderNonVisible, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE descriptorType,
    DescriptorAllocation& allocation)
{
    if (isUAVShaderNonVisible)
        Allocate(device, true, numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, usedUavShaderNonVisibleDescriptorHeapPages,
            emptyUavShaderNonVisibleDescriptorHeapPages, currentUavShaderNonVisibleDescriptorHeapPage, allocation);
    else if (descriptorType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
        Allocate(device, false, numDescriptors, descriptorType, usedCbvSrvUavDescriptorHeapPages, emptyCbvSrvUavDescriptorHeapPages,
            currentCbvSrvUavDescriptorHeapPage, allocation);
    else if (descriptorType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
        Allocate(device, false, numDescriptors, descriptorType, usedSamplerDescriptorHeapPages, emptySamplerDescriptorHeapPages,
            currentSamplerDescriptorHeapPage, allocation);
    else if (descriptorType == D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
        Allocate(device, false, numDescriptors, descriptorType, usedRtvDescriptorHeapPages, emptyRtvDescriptorHeapPages,
            currentRtvDescriptorHeapPage, allocation);
    else if (descriptorType == D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
        Allocate(device, false, numDescriptors, descriptorType, usedDsvDescriptorHeapPages, emptyDsvDescriptorHeapPages,
            currentDsvDescriptorHeapPage, allocation);
}

void Graphics::DescriptorAllocator::Allocate(ID3D12Device* device, bool isUAVShaderNonVisible, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE descriptorType,
    DescriptorHeapPool& usedHeapPool, DescriptorHeapPool& emptyHeapPool, std::shared_ptr<DescriptorAllocationPage>& currentPage,
    DescriptorAllocation& allocation)
{
    if (currentPage.get() == nullptr || !currentPage->HasSpace(numDescriptors))
        SetNewPageAsCurrent(device, isUAVShaderNonVisible, numDescriptors, descriptorType, usedHeapPool, emptyHeapPool, currentPage);

    currentPage->Allocate(numDescriptors, allocation);
}

void Graphics::DescriptorAllocator::SetNewPageAsCurrent(ID3D12Device* device, bool isUAVShaderNonVisible, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE descriptorType,
    DescriptorHeapPool& usedHeapPool, DescriptorHeapPool& emptyHeapPool, std::shared_ptr<DescriptorAllocationPage>& currentPage)
{
    if (emptyHeapPool.empty())
    {
        currentPage = std::shared_ptr<DescriptorAllocationPage>(new DescriptorAllocationPage(device, descriptorType, isUAVShaderNonVisible, numDescriptorsPerHeap));

        usedHeapPool.push_back(currentPage);
    }
    else
    {
        currentPage = emptyHeapPool.front();

        emptyHeapPool.pop_front();
    }
}
