#pragma once

#include "Mesh.h"
#include "Material.h"

namespace Graphics
{
	class GraphicObject
	{
	public:
		GraphicObject();
		~GraphicObject();

		void AssignMesh(const Mesh* newMesh);
		void AssignMaterial(const Material* newMaterial);

		const BoundingBox& GetBoundingBox() const;

		void Draw(ID3D12GraphicsCommandList* commandList) const;

	private:
		void DrawMesh(ID3D12GraphicsCommandList* commandList) const;

		using DrawFunction = void (Graphics::GraphicObject::*)(ID3D12GraphicsCommandList*) const;

		DrawFunction drawFunction;

		BoundingBox boundingBox;

		const Mesh* mesh;
		const Material* material;
	};
}
