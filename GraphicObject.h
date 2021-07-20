#pragma once

#include "Mesh.h"
#include "ParticleSystem.h"
#include "Cloth.h"
#include "Material.h"
#include "ComputeObject.h"
#include "IRenderable.h"

namespace Graphics
{
	enum class RenderingLayer
	{
		RENDERING_LAYER_OPAQUE,
		RENDERING_LAYER_TRANSPARENT,
		RENDERING_LAYER_EFFECT
	};

	class GraphicObject final
	{
	public:
		GraphicObject();
		virtual ~GraphicObject();

		void AssignRenderableEntity(const IRenderable* renderableEntity);
		void AssignMaterial(const Material* newMaterial);

		void SetRenderingLayer(RenderingLayer renderingLayer);

		const BoundingBox& GetBoundingBox() const noexcept;
		const RenderingLayer& GetRenderingLayer() const noexcept;

		void Execute(ID3D12GraphicsCommandList* commandList) const;
		void Draw(ID3D12GraphicsCommandList* commandList) const;

	private:
		BoundingBox boundingBox;

		RenderingLayer layer;

		const Material* material;
		const IRenderable* renderable;
	};
}
