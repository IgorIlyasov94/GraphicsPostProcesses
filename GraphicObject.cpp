#include "GraphicObject.h"

Graphics::GraphicObject::GraphicObject()
	: boundingBox{}, layer(RenderingLayer::RENDERING_LAYER_OPAQUE), renderable(nullptr), material(nullptr)
{

}

Graphics::GraphicObject::~GraphicObject()
{

}

void Graphics::GraphicObject::AssignRenderableEntity(const IRenderable* renderableEntity)
{
	renderable = renderableEntity;

	if (renderable != nullptr)
		boundingBox = renderable->GetBoundingBox();
}

void Graphics::GraphicObject::AssignMaterial(const Material* newMaterial)
{
	if (newMaterial != nullptr)
		if (!newMaterial->IsComposed())
			throw std::exception("Graphics::GraphicObject::AssignMaterial: Material is not composed");

	material = newMaterial;
}

void Graphics::GraphicObject::SetRenderingLayer(RenderingLayer renderingLayer)
{
	layer = renderingLayer;
}

const Graphics::BoundingBox& Graphics::GraphicObject::GetBoundingBox() const noexcept
{
	return boundingBox;
}

const Graphics::RenderingLayer& Graphics::GraphicObject::GetRenderingLayer() const noexcept
{
	return layer;
}

void Graphics::GraphicObject::Execute(ID3D12GraphicsCommandList* commandList) const
{
	if (renderable != nullptr)
		renderable->Update(commandList);
}

void Graphics::GraphicObject::Draw(ID3D12GraphicsCommandList* commandList) const
{
	if (renderable != nullptr)
		renderable->Draw(commandList, material);
}
