#pragma once

#include "PointLight.h"
#include "ResourceManager.h"
#include "GraphicsSettings.h"

namespace Graphics
{
	class LightingSystem
	{
	public:
		static LightingSystem& GetInstance();



	private:
		LightingSystem();
		~LightingSystem();

		LightingSystem(const LightingSystem&) = delete;
		LightingSystem(LightingSystem&&) = delete;
		LightingSystem& operator=(const LightingSystem&) = delete;
		LightingSystem& operator=(LightingSystem&&) = delete;

		std::vector<PointLight> pointLights;

		ResourceManager& resourceManager = ResourceManager::GetInstance();
	};
}
