#include "GraphicsSettings.h"

void Graphics::GraphicsSettings::SetStandard()
{
	resolutionX = 1920;
	resolutionY = 1080;

	framesPerSecond = 60;

	toggleFullscreen = false;
}

const uint32_t& Graphics::GraphicsSettings::GetResolutionX()
{
	return resolutionX;
}

const uint32_t& Graphics::GraphicsSettings::GetResolutionY()
{
	return resolutionY;
}

const uint32_t& Graphics::GraphicsSettings::GetFramesPerSecond()
{
	return framesPerSecond;
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

uint32_t Graphics::GraphicsSettings::resolutionX = 1920;
uint32_t Graphics::GraphicsSettings::resolutionY = 1080;

uint32_t Graphics::GraphicsSettings::framesPerSecond = 60;

bool Graphics::GraphicsSettings::toggleFullscreen = true;
