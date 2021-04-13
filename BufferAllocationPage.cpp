#include "BufferAllocationPage.h"

Graphics::BufferAllocationPage::BufferAllocationPage(ID3D12Device* device, D3D12_HEAP_TYPE _heapType, uint64_t _pageSize)
	: heapType(_heapType), pageSize(_pageSize), offset(0), currentCPUAddress(nullptr), currentGPUAddress(D3D12_GPU_VIRTUAL_ADDRESS(0))
{
	D3D12_HEAP_PROPERTIES heapProperties;
	SetupHeapProperties(heapProperties, heapType);

	D3D12_RESOURCE_DESC resourceDesc;
	SetupResourceBufferDesc(resourceDesc, _pageSize);

	D3D12_RESOURCE_STATES resourceState;

	if (heapType == D3D12_HEAP_TYPE_UPLOAD)
		resourceState = D3D12_RESOURCE_STATE_GENERIC_READ;
	else
		resourceState = D3D12_RESOURCE_STATE_COPY_DEST;

	ThrowIfFailed(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, resourceState, nullptr,
		IID_PPV_ARGS(&pageResource)), "BufferAllocationPage::BufferAllocationPage: Resource creating error");

	D3D12_RANGE range = { 0, 0 };

	if (heapType == D3D12_HEAP_TYPE_UPLOAD)
		ThrowIfFailed(pageResource->Map(0, &range, reinterpret_cast<void**>(&currentCPUAddress)),
			"BufferAllocationPage::BufferAllocationPage: Resource mapping error");

	currentGPUAddress = pageResource->GetGPUVirtualAddress();
}

Graphics::BufferAllocationPage::~BufferAllocationPage()
{
	if (heapType == D3D12_HEAP_TYPE_UPLOAD)
		pageResource->Unmap(0, nullptr);

	currentCPUAddress = nullptr;
	currentGPUAddress = D3D12_GPU_VIRTUAL_ADDRESS(0);

	offset = 0;
}

void Graphics::BufferAllocationPage::Allocate(uint64_t _size, uint64_t alignment, BufferAllocation& allocation)
{
	if (!HasSpace(_size, alignment))
		throw std::exception("BufferAllocationPage::Allocate: Bad allocation");

	auto alignedSize = AlignSize(_size, alignment);
	offset = AlignSize(offset, alignment);

	allocation.cpuAddress = currentCPUAddress + offset;
	allocation.gpuAddress = currentGPUAddress + offset;
	allocation.gpuPageOffset = offset;
	allocation.bufferResource = pageResource.Get();

	offset += alignedSize;
}

bool Graphics::BufferAllocationPage::HasSpace(uint64_t _size, uint64_t alignment) const noexcept
{
	auto alignedSize = AlignSize(_size, alignment);
	auto alignedOffset = AlignSize(offset, alignment);

	return alignedSize + alignedOffset <= pageSize;
}