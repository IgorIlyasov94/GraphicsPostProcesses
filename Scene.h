#pragma once

#include "GraphicObject.h"
#include "ComputeObject.h"
#include "Camera.h"
#include "Octree.h"
#include "LightingSystem.h"
#include "GraphicsSettings.h"

namespace Graphics
{
	class Scene
	{
	public:
		Scene();
		~Scene();

		LightingSystem* GetLightingSystem();

		void SetMainCamera(const Camera* camera);
		void EmplaceComputeObject(const ComputeObject* object);
		void EmplaceGraphicObject(const GraphicObject* object, bool isDynamic);

		void ExecuteScripts(ID3D12GraphicsCommandList* commandList);
		void Draw(ID3D12GraphicsCommandList* commandList) const;

	private:
		const Camera* mainCamera;

		ObjectPtrPool visibleObjectsList;
		ObjectPtrPool visibleTransparentObjectsList;
		ObjectPtrPool visibleEffectObjectsList;
		std::vector<const ComputeObject*> computeObjects;

		std::shared_ptr<Octree> octree;

		std::shared_ptr<LightingSystem> lightingSystem;
	};
}
