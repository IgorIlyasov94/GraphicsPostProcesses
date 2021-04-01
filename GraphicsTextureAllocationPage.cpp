#include "GraphicsTextureAllocationPage.h"

GraphicsTextureAllocationPage::GraphicsTextureAllocationPage(ID3D12Device* device, D3D12_HEAP_TYPE _heapType, D3D12_RESOURCE_FLAGS resourceFlags,
	const TextureInfo& textureInfo)
	: cpuAddress(nullptr), gpuAddress(D3D12_GPU_VIRTUAL_ADDRESS(0)), heapType(_heapType)
{
	D3D12_HEAP_PROPERTIES heapProperties;
	SetupHeapProperties(heapProperties, heapType);

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = textureInfo.dimension;
	resourceDesc.DepthOrArraySize = textureInfo.depth;
	resourceDesc.Alignment = 0;
	resourceDesc.Flags = resourceFlags;
	resourceDesc.Format = textureInfo.format;
	resourceDesc.Height = textureInfo.height;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.MipLevels = textureInfo.mipLevels;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Width = textureInfo.width;

	D3D12_RESOURCE_STATES resourceState;

	if (heapType == D3D12_HEAP_TYPE_UPLOAD)
		resourceState = D3D12_RESOURCE_STATE_GENERIC_READ;
	else
		resourceState = D3D12_RESOURCE_STATE_COPY_DEST;

	ThrowIfFailed(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, resourceState, nullptr,
		IID_PPV_ARGS(&pageResource)), "GraphicsTextureAllocationPage::GraphicsTextureAllocationPage: Resource creating error");

	D3D12_RANGE range = { 0, 0 };

	if (heapType == D3D12_HEAP_TYPE_UPLOAD)
		ThrowIfFailed(pageResource->Map(0, &range, reinterpret_cast<void**>(&cpuAddress)),
			"GraphicsTextureAllocationPage::GraphicsTextureAllocationPage: Resource mapping error");

	gpuAddress = pageResource->GetGPUVirtualAddress();
}

GraphicsTextureAllocationPage::~GraphicsTextureAllocationPage()
{
	if (heapType == D3D12_HEAP_TYPE_UPLOAD)
		pageResource->Unmap(0, nullptr);

	cpuAddress = nullptr;
	gpuAddress = D3D12_GPU_VIRTUAL_ADDRESS(0);
}

void GraphicsTextureAllocationPage::GetAllocation(GraphicsTextureAllocation& allocation)
{
	allocation.cpuAddress = cpuAddress;
	allocation.gpuAddress = gpuAddress;
	allocation.textureResource = pageResource.Get();
}
