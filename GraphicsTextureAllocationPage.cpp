#include "GraphicsTextureAllocationPage.h"

GraphicsTextureAllocationPage::GraphicsTextureAllocationPage(ID3D12Device* device, D3D12_HEAP_TYPE _heapType, D3D12_RESOURCE_FLAGS resourceFlags,
	const TextureInfo& textureInfo)
	: cpuAddress(nullptr), heapType(_heapType)
{
	D3D12_HEAP_PROPERTIES heapProperties;
	SetupHeapProperties(heapProperties, heapType);

	D3D12_RESOURCE_DESC resourceDesc{};

	D3D12_RESOURCE_STATES resourceState;

	if (heapType == D3D12_HEAP_TYPE_UPLOAD)
	{
		D3D12_RESOURCE_DESC destResourceDesc{};

		SetupResourceTextureDesc(destResourceDesc, textureInfo, resourceFlags);

		uint64_t requiredUploadBufferSize;
		uint32_t numSubresources = textureInfo.depth * textureInfo.mipLevels;

		device->GetCopyableFootprints(&destResourceDesc, 0, numSubresources, 0, nullptr, nullptr, nullptr, &requiredUploadBufferSize);

		SetupResourceBufferDesc(resourceDesc, requiredUploadBufferSize);
		resourceState = D3D12_RESOURCE_STATE_GENERIC_READ;
	}
	else
	{
		SetupResourceTextureDesc(resourceDesc, textureInfo, resourceFlags);
		resourceState = D3D12_RESOURCE_STATE_COMMON;
	}

	ThrowIfFailed(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, resourceState, nullptr,
		IID_PPV_ARGS(&pageResource)), "GraphicsTextureAllocationPage::GraphicsTextureAllocationPage: Resource creating error");

	D3D12_RANGE range = { 0, 0 };

	if (heapType == D3D12_HEAP_TYPE_UPLOAD)
		ThrowIfFailed(pageResource->Map(0, &range, reinterpret_cast<void**>(&cpuAddress)),
			"GraphicsTextureAllocationPage::GraphicsTextureAllocationPage: Resource mapping error");
}

GraphicsTextureAllocationPage::~GraphicsTextureAllocationPage()
{
	if (heapType == D3D12_HEAP_TYPE_UPLOAD)
		pageResource->Unmap(0, nullptr);

	cpuAddress = nullptr;
}

void GraphicsTextureAllocationPage::GetAllocation(GraphicsTextureAllocation& allocation)
{
	allocation.cpuAddress = cpuAddress;
	allocation.textureResource = pageResource.Get();
}
