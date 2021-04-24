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

		void EmplaceGraphicObject(const GraphicObject* object, bool isDynamic);

		void Draw(ID3D12GraphicsCommandList* commandList) const;

	private:
		std::shared_ptr<Camera> camera;
		std::shared_ptr<Octree> octree;
	};
}
