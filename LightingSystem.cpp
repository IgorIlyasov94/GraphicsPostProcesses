#include "LightingSystem.h"

Graphics::LightingSystem& Graphics::LightingSystem::GetInstance()
{
	static LightingSystem instance;

	return instance;
}

Graphics::PointLightId Graphics::LightingSystem::CreatePointLight(float3 position, float3 color, float radius, float intensity, bool isShadowCaster)
{
	pointLights.push_back(PointLight(position, color, radius, intensity, isShadowCaster));

	return PointLightId(pointLights.size() - 1);
}

Graphics::PointLightId Graphics::LightingSystem::CreatePointLight(PointLight&& pointLight)
{
	pointLights.push_back(std::forward<PointLight>(pointLight));

	return PointLightId(pointLights.size() - 1);
}

Graphics::PointLight& Graphics::LightingSystem::SetPointLight(const PointLightId& pointLightId)
{
	return pointLights[pointLightId.value];
}

void Graphics::LightingSystem::Clear()
{
	pointLights.clear();
}

Graphics::LightingSystem::LightingSystem()
{

}

Graphics::LightingSystem::~LightingSystem()
{

}
