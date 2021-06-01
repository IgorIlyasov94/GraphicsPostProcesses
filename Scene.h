#pragma once

#include "GraphicObject.h"
#include "ComputeObject.h"
#include "Camera.h"
#include "Octree.h"
#include "GraphicsSettings.h"

namespace Graphics
{
	class Scene
	{
	public:
		Scene();
		~Scene();

		void SetMainCamera(const Camera* camera);
		void EmplaceComputeObject(const ComputeObject* object);
		void EmplaceGraphicObject(const GraphicObject* object, bool isDynamic);

		void ExecuteScripts(ID3D12GraphicsCommandList* commandList);
		void Draw(ID3D12GraphicsCommandList* commandList) const;

	private:
		const Camera* mainCamera;

		std::vector<const GraphicObject*> visibleObjectsList;
		std::vector<const ComputeObject*> computeObjects;

		std::shared_ptr<Octree> octree;
	};
}
