#include "GraphicsDescriptorAllocator.h"

GraphicsDescriptorAllocator& GraphicsDescriptorAllocator::GetInstance()
{
    static GraphicsDescriptorAllocator thisInstance;

    return thisInstance;
}

void GraphicsDescriptorAllocator::Allocate(ID3D12Device* device, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE descriptorType,
    GraphicsDescriptorAllocation& allocation)
{
    if (descriptorType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
        Allocate(device, numDescriptors, descriptorType, usedCbvSrvUavDescriptorHeapPages, emptyCbvSrvUavDescriptorHeapPages,
            currentCbvSrvUavDescriptorHeapPage, allocation);
    else if (descriptorType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
        Allocate(device, numDescriptors, descriptorType, usedSamplerDescriptorHeapPages, emptySamplerDescriptorHeapPages,
            currentSamplerDescriptorHeapPage, allocation);
    else if (descriptorType == D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
        Allocate(device, numDescriptors, descriptorType, usedRtvDescriptorHeapPages, emptyRtvDescriptorHeapPages,
            currentRtvUavDescriptorHeapPage, allocation);
    else if (descriptorType == D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
        Allocate(device, numDescriptors, descriptorType, usedDsvDescriptorHeapPages, emptyDsvDescriptorHeapPages,
            currentDsvDescriptorHeapPage, allocation);
}

void GraphicsDescriptorAllocator::Allocate(ID3D12Device* device, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE descriptorType,
    DescriptorHeapPool& usedHeapPool, DescriptorHeapPool& emptyHeapPool, std::shared_ptr<GraphicsDescriptorAllocationPage>& currentPage,
    GraphicsDescriptorAllocation& allocation)
{
    if (currentPage.get() == nullptr || !currentPage->HasSpace(numDescriptors))
        SetNewPageAsCurrent(device, numDescriptors, descriptorType, usedHeapPool, emptyHeapPool, currentPage);

    currentPage->Allocate(numDescriptors, allocation);
}

void GraphicsDescriptorAllocator::SetNewPageAsCurrent(ID3D12Device* device, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE descriptorType,
    DescriptorHeapPool& usedHeapPool, DescriptorHeapPool& emptyHeapPool, std::shared_ptr<GraphicsDescriptorAllocationPage>& currentPage)
{
    if (emptyHeapPool.empty())
    {
        currentPage = std::make_shared<GraphicsDescriptorAllocationPage>(device, descriptorType, numDescriptorsPerHeap);

        usedHeapPool.push_back(currentPage);
    }
    else
    {
        currentPage = emptyHeapPool.front();

        emptyHeapPool.pop_front();
    }
}
