#include "ParticleSystemManager.h"

Graphics::ParticleSystemManager& Graphics::ParticleSystemManager::GetInstance()
{
	static ParticleSystemManager instance;

	return instance;
}

Graphics::ParticleSystemManager::ParticleSystemManager()
{

}

Graphics::ParticleSystemManager::~ParticleSystemManager()
{

}
