#pragma once

#include "GraphicsHelper.h"

class GraphicsDescriptorAllocationPage;

struct GraphicsDescriptorAllocation
{
public:
	GraphicsDescriptorAllocation();
	~GraphicsDescriptorAllocation();

	bool IsNull() const;

private:
	D3D12_CPU_DESCRIPTOR_HANDLE descriptorBase;
};