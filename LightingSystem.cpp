#include "LightingSystem.h"

Graphics::LightingSystem& Graphics::LightingSystem::GetInstance()
{
	static LightingSystem instance;

	return instance;
}

Graphics::LightingSystem::LightingSystem()
{

}

Graphics::LightingSystem::~LightingSystem()
{

}
