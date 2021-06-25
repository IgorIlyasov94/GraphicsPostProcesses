#pragma once

#include "ResourceManager.h"
#include "Material.h"
#include "ComputeObject.h"

namespace Graphics
{
	using ParticleSystemId = ResourceId<30>;

	class ParticleSystemManager
	{
	public:
		static ParticleSystemManager& GetInstance();



	private:
		ParticleSystemManager();
		~ParticleSystemManager();

		ParticleSystemManager(const ParticleSystemManager&) = delete;
		ParticleSystemManager(ParticleSystemManager&&) = delete;
		ParticleSystemManager& operator=(const ParticleSystemManager&) = delete;
		ParticleSystemManager& operator=(ParticleSystemManager&&) = delete;

		ResourceManager& resourceManager = ResourceManager::GetInstance();
	};
}
