#include "PointLight.h"

Graphics::PointLight::PointLight()
	: position{}, color{}, radius{}, intensity{}, isShadowCaster(false)
{

}

Graphics::PointLight::PointLight(float3 _position, float3 _color, float _radius, float _intensity, bool _isShadowCaster)
	: position(_position), color(_color), radius(_radius), intensity(_intensity), isShadowCaster(_isShadowCaster)
{

}

Graphics::PointLight::~PointLight()
{

}

const float3& Graphics::PointLight::GetPosition() const
{
	return position;
}

const float3& Graphics::PointLight::GetColor() const
{
	return color;
}

const float& Graphics::PointLight::GetRadius() const
{
	return radius;
}

const float& Graphics::PointLight::GetIntensity() const
{
	return intensity;
}

const bool& Graphics::PointLight::IsShadowCaster() const
{
	return isShadowCaster;
}

void Graphics::PointLight::Move(float3 newPosition)
{
	position = newPosition;
}

void Graphics::PointLight::SetColor(float3 newColor)
{
	color = newColor;
}

void Graphics::PointLight::SetRadius(float newRadius)
{
	radius = newRadius;
}

void Graphics::PointLight::SetIntensity(float newIntensity)
{
	intensity = newIntensity;
}

void Graphics::PointLight::SetShadowCasting(bool castShadows)
{
	isShadowCaster = castShadows;
}
