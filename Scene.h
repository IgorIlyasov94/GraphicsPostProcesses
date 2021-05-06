#pragma once

#include "GraphicObject.h"
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
		void EmplaceGraphicObject(const GraphicObject* object, bool isDynamic);

		void Draw(ID3D12GraphicsCommandList* commandList) const;

	private:
		const Camera* mainCamera;

		std::shared_ptr<Octree> octree;
	};
}
