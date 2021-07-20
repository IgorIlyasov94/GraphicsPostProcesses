#pragma once

#include "ResourceManager.h"
#include "Mesh.h"
#include "Material.h"
#include "ComputeObject.h"
#include "IRenderable.h"

namespace Graphics
{
	class Cloth final : public IRenderable
	{
	public:
		Cloth(ID3D12GraphicsCommandList* commandList, const Mesh* _mesh);
		~Cloth();

		const BoundingBox& GetBoundingBox() const noexcept override;

		void Update(ID3D12GraphicsCommandList* commandList) const override;
		void Draw(ID3D12GraphicsCommandList* commandList, const Material* material) const override;

	private:
		Cloth() = delete;

		BoundingBox boundingBox;

		const Mesh* mesh;
		const Material* material;

		ResourceManager& resourceManager = ResourceManager::GetInstance();
	};
}
