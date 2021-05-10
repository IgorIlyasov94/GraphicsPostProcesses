#pragma once

#include "GraphicsHelper.h"

namespace Graphics
{
	class GraphicsSettings
	{
	public:
		static void SetStandard();

		static const uint32_t& GetResolutionX();
		static const uint32_t& GetResolutionY();

		static const bool& IsFullscreen();

	private:
		GraphicsSettings();
		~GraphicsSettings();

		static uint32_t resolutionX;
		static uint32_t resolutionY;
		static bool toggleFullscreen;
	};
}
