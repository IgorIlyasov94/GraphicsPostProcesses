#include "GraphicsDescriptorAllocator.h"

GraphicsDescriptorAllocator& GraphicsDescriptorAllocator::GetInstance()
{
    static GraphicsDescriptorAllocator thisInstance;

    return thisInstance;
}

void GraphicsDescriptorAllocator::Allocate(ID3D12Device* device, const uint32_t numDescriptors, const D3D12_DESCRIPTOR_HEAP_TYPE descriptorType,
    GraphicsDescriptorAllocation& allocation)
{
    std::lock_guard<std::mutex> lock(descriptorAllocatorMutex);

    switch (descriptorType)
    {
    case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
        Allocate(device, numDescriptors, descriptorType, cbvSrvUavDescriptorHeapPages, availableCbvSrvUavHeapIndices, allocation);
        break;

    case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
        Allocate(device, numDescriptors, descriptorType, samplerDescriptorHeapPages, availableSamplerHeapIndices, allocation);
        break;

    case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
        Allocate(device, numDescriptors, descriptorType, rtvDescriptorHeapPages, availableCbvSrvUavHeapIndices, allocation);
        break;

    case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
        Allocate(device, numDescriptors, descriptorType, dsvDescriptorHeapPages, availableCbvSrvUavHeapIndices, allocation);
        break;

    default:
        throw std::exception("GraphicsDescriptorAllocator::Allocate: Wrong heap type!");
    }
}

void GraphicsDescriptorAllocator::Allocate(ID3D12Device* device, const uint32_t numDescriptors, const D3D12_DESCRIPTOR_HEAP_TYPE descriptorType,
    DescriptorHeapPool& heapPool, std::set<size_t>& availableHeapIndices, GraphicsDescriptorAllocation& allocation)
{
    for (auto descriptorIdIterator = availableHeapIndices.begin(); descriptorIdIterator != availableHeapIndices.end(); descriptorIdIterator++)
    {
        auto& allocationPage = heapPool[*descriptorIdIterator];

        allocationPage->Allocate(numDescriptors, allocation);

        if (allocationPage->GetNumFreeHandles() == 0)
        {
            descriptorIdIterator = availableHeapIndices.erase(descriptorIdIterator);
        }

        if (!allocation.IsNull())
            break;
    }

    if (allocation.IsNull())
    {
        numDescriptorsPerHeap = std::max(numDescriptorsPerHeap, numDescriptors);

        auto newPage = std::make_shared<GraphicsDescriptorAllocationPage>(device, descriptorType, numDescriptorsPerHeap);

        heapPool.emplace_back(newPage);
        availableHeapIndices.insert(heapPool.size() - 1);

        newPage->Allocate(numDescriptors, allocation);
    }
}
