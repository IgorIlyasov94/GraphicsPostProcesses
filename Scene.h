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

		void Draw(ID3D12GraphicsCommandList* commandList) const;

	private:
		std::unique_ptr<Camera> camera;
		std::unique_ptr<Octree> octree;
	};
}
