#include "Mesh.h"

Graphics::Mesh::Mesh(std::filesystem::path filePath)
	: indicesCount(0), vertexBufferView(nullptr), indexBufferView(nullptr)
{
	std::vector<uint8_t> verticesData;
	std::vector<uint8_t> indicesData;
	VertexFormat vertexFormat;

	Graphics::OBJLoader::Load(filePath, true, false, vertexFormat, verticesData, indicesData);

	auto vertexStride = GetVertexStrideFromFormat(vertexFormat);
	vertexBufferId = resourceManager.CreateVertexBuffer(verticesData, vertexStride);
	indexBufferId = resourceManager.CreateIndexBuffer(indicesData, 4);

	indicesCount = resourceManager.GetIndexBuffer(indexBufferId).indicesCount;

	vertexBufferView = &resourceManager.GetVertexBuffer(vertexBufferId).vertexBufferView;
	indexBufferView = &resourceManager.GetIndexBuffer(indexBufferId).indexBufferView;
}

Graphics::Mesh::Mesh(VertexFormat vertexFormat, std::vector<uint8_t> verticesData, std::vector<uint8_t> indicesData)
	: indicesCount(0), vertexBufferView(nullptr), indexBufferView(nullptr)
{
	auto vertexStride = GetVertexStrideFromFormat(vertexFormat);
	vertexBufferId = resourceManager.CreateVertexBuffer(verticesData, vertexStride);
	indexBufferId = resourceManager.CreateIndexBuffer(indicesData, 4);

	indicesCount = resourceManager.GetIndexBuffer(indexBufferId).indicesCount;

	vertexBufferView = &resourceManager.GetVertexBuffer(vertexBufferId).vertexBufferView;
	indexBufferView = &resourceManager.GetIndexBuffer(indexBufferId).indexBufferView;
}

Graphics::Mesh::~Mesh()
{

}

uint32_t Graphics::Mesh::GetIndicesCount()
{
	return indicesCount;
}

void Graphics::Mesh::Present(ID3D12GraphicsCommandList* commandList)
{
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	commandList->IASetVertexBuffers(0, 1, vertexBufferView);
	commandList->IASetIndexBuffer(indexBufferView);
}
