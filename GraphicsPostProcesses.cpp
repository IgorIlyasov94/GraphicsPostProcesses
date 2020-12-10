#include "GraphicsPostProcesses.h"

GraphicsPostProcesses::GraphicsPostProcesses()
{

}

GraphicsPostProcesses& GraphicsPostProcesses::getInstance()
{
	static GraphicsPostProcesses instance;

	return instance;
}

void GraphicsPostProcesses::initialize(const int32_t& resolutionX, const int32_t& resolutionY, const ID3D12Device*& device)
{

}

void GraphicsPostProcesses::enableHDR()
{

}