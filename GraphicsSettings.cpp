#include "GraphicsSettings.h"

void Graphics::GraphicsSettings::SetStandard()
{
	resolutionX = 1320;
	resolutionY = 990;
	toggleFullscreen = true;
}

const uint32_t& Graphics::GraphicsSettings::GetResolutionX()
{
	return resolutionX;
}

const uint32_t& Graphics::GraphicsSettings::GetResolutionY()
{
	return resolutionY;
}

const bool& Graphics::GraphicsSettings::IsFullscreen()
{
	return toggleFullscreen;
}

Graphics::GraphicsSettings::GraphicsSettings()
{

}

Graphics::GraphicsSettings::~GraphicsSettings()
{

}

uint32_t Graphics::GraphicsSettings::resolutionX = 1320;
uint32_t Graphics::GraphicsSettings::resolutionY = 990;
bool Graphics::GraphicsSettings::toggleFullscreen = false;
