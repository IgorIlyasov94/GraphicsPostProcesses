#include "GraphicObject.h"

Graphics::GraphicObject::GraphicObject()
	: drawFunction(nullptr), boundingBox{}, mesh(nullptr), material(nullptr), computeObject(nullptr)
{

}

Graphics::GraphicObject::~GraphicObject()
{

}

void Graphics::GraphicObject::AssignMesh(const Mesh* newMesh)
{
	mesh = newMesh;

	if (mesh != nullptr)
	{
		boundingBox = mesh->GetBoundingBox();

		drawFunction = &Graphics::GraphicObject::DrawMesh;
	}
}

void Graphics::GraphicObject::AssignParticleSystem(const ParticleSystem* newParticleSystem)
{
	particleSystem = newParticleSystem;

	if (particleSystem != nullptr)
	{
		boundingBox = particleSystem->GetBoundingBox();


	}
}

void Graphics::GraphicObject::AssignMaterial(const Material* newMaterial)
{
	if (newMaterial != nullptr)
		if (!newMaterial->IsComposed())
			throw std::exception("Graphics::GraphicObject::AssignMaterial: Material is not composed");

	material = newMaterial;
}

void Graphics::GraphicObject::AssignComputeObject(const ComputeObject* newComputeObject)
{
	if (newComputeObject != nullptr)
		if (!newComputeObject->IsComposed())
			throw std::exception("Graphics::GraphicObject::AssignComputeObject: ComputeObject is not composed");

	computeObject = newComputeObject;
}

const Graphics::BoundingBox& Graphics::GraphicObject::GetBoundingBox() const noexcept
{
	return boundingBox;
}

void Graphics::GraphicObject::Execute(ID3D12GraphicsCommandList* commandList) const
{
	if (computeObject == nullptr)
		return;

	computeObject->Present(commandList);
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

void Graphics::GraphicObject::DrawParticleSystem(ID3D12GraphicsCommandList* commandList) const
{
	if (particleSystem == nullptr)
		return;

	particleSystem->Present(commandList);

	commandList->DrawIndexedInstanced(particleSystem->GetParticleMaxCount(), 1, 0, 0, 0);
}
