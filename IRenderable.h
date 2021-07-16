#pragma once

#include "Material.h"

namespace Graphics
{
	class IRenderable
	{
	public:
		virtual ~IRenderable() {};

		virtual const BoundingBox& GetBoundingBox() const noexcept = 0;

		virtual void Update(ID3D12GraphicsCommandList* commandList) const = 0;
		virtual void Draw(ID3D12GraphicsCommandList* commandList, const Material* material) const = 0;
	};
}