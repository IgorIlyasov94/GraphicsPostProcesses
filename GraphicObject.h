#pragma once

#include "Mesh.h"
#include "ParticleSystem.h"
#include "Material.h"
#include "ComputeObject.h"

namespace Graphics
{
	enum class RenderingLayer
	{
		RENDERING_LAYER_OPAQUE,
		RENDERING_LAYER_TRANSPARENT,
		RENDERING_LAYER_EFFECT
	};

	class GraphicObject
	{
	public:
		GraphicObject();
		~GraphicObject();

		void AssignMesh(const Mesh* newMesh);
		void AssignParticleSystem(const ParticleSystem* newParticleSystem);
		void AssignMaterial(const Material* newMaterial);
		void AssignComputeObject(const ComputeObject* newComputeObject);

		void SetRenderingLayer(RenderingLayer renderingLayer);

		const BoundingBox& GetBoundingBox() const noexcept;
		const RenderingLayer& GetRenderingLayer() const noexcept;

		void Execute(ID3D12GraphicsCommandList* commandList) const;
		void Draw(ID3D12GraphicsCommandList* commandList) const;

	private:
		void DrawMesh(ID3D12GraphicsCommandList* commandList) const;
		void DrawParticleSystem(ID3D12GraphicsCommandList* commandList) const;

		using DrawFunction = void (Graphics::GraphicObject::*)(ID3D12GraphicsCommandList*) const;

		DrawFunction drawFunction;

		BoundingBox boundingBox;

		RenderingLayer layer;

		const Mesh* mesh;
		const ParticleSystem* particleSystem;
		const Material* material;
		const ComputeObject* computeObject;
	};
}
