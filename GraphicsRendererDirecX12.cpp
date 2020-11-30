#include "GraphicsRendererDirectX12.h"

GraphicsRendererDirectX12::GraphicsRendererDirectX12()
	: fenceEvent(nullptr), gpuMemory(0), bufferIndex(0), fenceValue(0),
	resolutionX(1024), resolutionY(768)
{

}

GraphicsRendererDirectX12& GraphicsRendererDirectX12::getInstance()
{
	static GraphicsRendererDirectX12 instance;

	return instance;
}

inline ID3D12Device* const& GraphicsRendererDirectX12::getDevice()
{
	return device.Get();
}

int32_t const& GraphicsRendererDirectX12::getResolutionX() const
{
	return resolutionX;
}

int32_t const& GraphicsRendererDirectX12::getResolutionY() const
{
	return resolutionY;
}

void GraphicsRendererDirectX12::initialize()
{

}

void GraphicsRendererDirectX12::gpuRelease()
{
}

void GraphicsRendererDirectX12::frameRender()
{
}
