#pragma once

#include "GraphicsHelper.h"

namespace Graphics
{
	class PointLight
	{
	public:
		PointLight();
		PointLight(float3 _position, float3 _color, float _radius, float _intensity, bool _isShadowCaster);
		~PointLight();

		const float3& GetPosition();
		const float3& GetColor();
		const float& GetRadius();
		const float& GetIntensity();

		const bool& IsShadowCaster();

		void Move(float3 newPosition);

		void SetColor(float3 newColor);
		void SetRadius(float newRadius);
		void SetIntensity(float newIntensity);
		void SetShadowCasting(bool castShadows);

	private:
		float3 position;
		float3 color;
		float radius;
		float intensity;

		bool isShadowCaster;
	};
}
