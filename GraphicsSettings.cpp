#include "GraphicsSettings.h"

void Graphics::GraphicsSettings::SetStandard()
{
	resolutionX = 1024;
	resolutionY = 768;
}

const uint32_t& Graphics::GraphicsSettings::GetResolutionX()
{
	return resolutionX;
}

const uint32_t& Graphics::GraphicsSettings::GetResolutionY()
{
	return resolutionY;
}

Graphics::GraphicsSettings::GraphicsSettings()
{

}

Graphics::GraphicsSettings::~GraphicsSettings()
{

}

uint32_t Graphics::GraphicsSettings::resolutionX = 1024;
uint32_t Graphics::GraphicsSettings::resolutionY = 768;
