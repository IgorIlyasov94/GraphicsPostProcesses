#include "Cloth.h"

Graphics::Cloth::Cloth(ID3D12GraphicsCommandList* commandList, const Mesh* _mesh)
	: boundingBox{}, mesh(nullptr), material(nullptr)
{
	mesh = _mesh;

	if (mesh == nullptr)
		throw std::exception("Graphics::Cloth::Cloth: Mesh is null");

	//boundingBox = mesh->GetBoundingBox();
	
	auto boundingBoxSize = BoundingBoxSize(boundingBox);

	XMStoreFloat3(&boundingBox.minCornerPoint, XMLoadFloat3(&boundingBox.minCornerPoint) - XMLoadFloat3(&boundingBoxSize));
	XMStoreFloat3(&boundingBox.maxCornerPoint, XMLoadFloat3(&boundingBox.maxCornerPoint) + XMLoadFloat3(&boundingBoxSize));

	/*std::vector<uint8_t> rawVerticesData;
	resourceManager.GetBufferDataFromGPU(mesh->GetVertexBufferId(), rawVerticesData);*/


}

Graphics::Cloth::~Cloth()
{

}

const Graphics::BoundingBox& Graphics::Cloth::GetBoundingBox() const noexcept
{
	return boundingBox;
}

void Graphics::Cloth::Update(ID3D12GraphicsCommandList* commandList) const
{
	//Сделать создание буфера связей через геометрический шейдер + UAV
}

void Graphics::Cloth::Draw(ID3D12GraphicsCommandList* commandList, const Material* material) const
{

}
