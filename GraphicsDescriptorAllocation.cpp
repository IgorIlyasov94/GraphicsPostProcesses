#include "GraphicsDescriptorAllocation.h"

GraphicsDescriptorAllocation::GraphicsDescriptorAllocation()
	: descriptorBase(D3D12_CPU_DESCRIPTOR_HANDLE{})
{

}

GraphicsDescriptorAllocation::~GraphicsDescriptorAllocation()
{

}

bool GraphicsDescriptorAllocation::IsNull() const
{
	return descriptorBase.ptr == 0;
}
