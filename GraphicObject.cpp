#include "GraphicObject.h"

Graphics::GraphicObject::GraphicObject()
	: drawFunction(nullptr), mesh(nullptr), material(nullptr)
{

}

Graphics::GraphicObject::~GraphicObject()
{

}

void Graphics::GraphicObject::AssignMesh(const Mesh* newMesh)
{
	mesh = newMesh;

	if (mesh != nullptr)
		drawFunction = &Graphics::GraphicObject::DrawMesh;
}

void Graphics::GraphicObject::AssignMaterial(const Material* newMaterial)
{
	if (newMaterial != nullptr)
		if (!newMaterial->IsComposed())
			throw std::exception("Graphics::GraphicObject::AssignMaterial: Material is not composed");

	material = newMaterial;
}

void Graphics::GraphicObject::Draw(ID3D12GraphicsCommandList* commandList) const
{
	if (material == nullptr || drawFunction == nullptr)
		return;

	material->Present(commandList);

	(this->*drawFunction)(commandList);
}

void Graphics::GraphicObject::DrawMesh(ID3D12GraphicsCommandList* commandList) const
{
	if (mesh == nullptr)
		return;

	mesh->Present(commandList);

	commandList->DrawIndexedInstanced(mesh->GetIndicesCount(), 1, 0, 0, 0);
}
