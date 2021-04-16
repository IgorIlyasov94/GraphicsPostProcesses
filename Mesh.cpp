#include "Mesh.h"

Graphics::Mesh::Mesh(std::filesystem::path filePath)
	: indicesCount(0), vertexBufferView(nullptr), indexBufferView(nullptr)
{
	std::vector<uint8_t> verticesData;
	std::vector<uint8_t> indicesData;
	VertexFormat vertexFormat;

	Graphics::OBJLoader::Load(filePath, true, false, vertexFormat, verticesData, indicesData);

	auto vertexStride = GetVertexStrideFromFormat(vertexFormat);
	vertexBufferId = resourceManager.CreateVertexBuffer(verticesData.data(), verticesData.size(), vertexStride);
	indexBufferId = resourceManager.CreateIndexBuffer(indicesData.data(), indicesData.size(), 4);

	indicesCount = resourceManager.GetIndexBuffer(indexBufferId).indicesCount;

	vertexBufferView = &resourceManager.GetVertexBuffer(vertexBufferId).vertexBufferView;
	indexBufferView = &resourceManager.GetIndexBuffer(indexBufferId).indexBufferView;
}

Graphics::Mesh::Mesh(VertexFormat vertexFormat, const void* verticesData, size_t verticesDataSize, const void* indicesData, size_t indicesDataSize)
	: indicesCount(0), vertexBufferView(nullptr), indexBufferView(nullptr)
{
	auto vertexStride = GetVertexStrideFromFormat(vertexFormat);
	vertexBufferId = resourceManager.CreateVertexBuffer(verticesData, verticesDataSize, vertexStride);
	indexBufferId = resourceManager.CreateIndexBuffer(indicesData, indicesDataSize, 4);

	indicesCount = resourceManager.GetIndexBuffer(indexBufferId).indicesCount;

	vertexBufferView = &resourceManager.GetVertexBuffer(vertexBufferId).vertexBufferView;
	indexBufferView = &resourceManager.GetIndexBuffer(indexBufferId).indexBufferView;
}

Graphics::Mesh::~Mesh()
{

}

uint32_t Graphics::Mesh::GetIndicesCount() const noexcept
{
	return indicesCount;
}

void Graphics::Mesh::Present(ID3D12GraphicsCommandList* commandList) const
{
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, vertexBufferView);
	commandList->IASetIndexBuffer(indexBufferView);
}
