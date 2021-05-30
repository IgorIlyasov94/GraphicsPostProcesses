#pragma once

#include "PointLight.h"
#include "ResourceManager.h"
#include "GraphicsSettings.h"

namespace Graphics
{
	template<uint8_t Category>
	class LightId
	{
	public:
		LightId()
			: value(0), category(Category)
		{

		}

		LightId(uint32_t resourceId)
			: value(resourceId), category(Category)
		{

		}

		uint32_t value;

	private:
		friend class LightingSystem;

		uint8_t category;
	};

	using PointLightId = typename LightId<0>;

	class LightingSystem
	{
	public:
		static LightingSystem& GetInstance();

		PointLightId CreatePointLight(float3 position, float3 color, float radius, float intensity, bool isShadowCaster);
		PointLightId CreatePointLight(PointLight&& pointLight);

		PointLight& SetPointLight(const PointLightId& pointLightId);
		
		void Clear();

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
