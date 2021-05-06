#include "Mesh.h"

Graphics::Mesh::Mesh(std::filesystem::path filePath)
	: indicesCount(0), vertexFormat(VertexFormat::UNDEFINED), boundingBox{}, vertexBufferView{}, indexBufferView{}
{
	std::vector<uint8_t> verticesData;
	std::vector<uint8_t> indicesData;
	
	Graphics::OBJLoader::Load(filePath, true, false, false, vertexFormat, verticesData, indicesData);

	FindBoundingBox(verticesData.data(), verticesData.size(), vertexFormat, boundingBox);
	
	auto vertexStride = GetVertexStride(vertexFormat);
	vertexBufferId = resourceManager.CreateVertexBuffer(verticesData.data(), verticesData.size(), vertexStride);
	indexBufferId = resourceManager.CreateIndexBuffer(indicesData.data(), indicesData.size(), 4);

	indicesCount = resourceManager.GetIndexBuffer(indexBufferId).indicesCount;

	vertexBufferView = resourceManager.GetVertexBufferView(vertexBufferId);
	indexBufferView = resourceManager.GetIndexBufferView(indexBufferId);

}

Graphics::Mesh::Mesh(VertexFormat _vertexFormat, const void* verticesData, size_t verticesDataSize, const void* indicesData, size_t indicesDataSize)
	: indicesCount(0), vertexFormat(_vertexFormat), boundingBox{}, vertexBufferView{}, indexBufferView{}
{
	FindBoundingBox(verticesData, verticesDataSize, vertexFormat, boundingBox);

	auto vertexStride = GetVertexStride(vertexFormat);
	vertexBufferId = resourceManager.CreateVertexBuffer(verticesData, verticesDataSize, vertexStride);
	indexBufferId = resourceManager.CreateIndexBuffer(indicesData, indicesDataSize, 4);

	indicesCount = resourceManager.GetIndexBuffer(indexBufferId).indicesCount;

	vertexBufferView = resourceManager.GetVertexBufferView(vertexBufferId);
	indexBufferView = resourceManager.GetIndexBufferView(indexBufferId);
}

Graphics::Mesh::~Mesh()
{

}

uint32_t Graphics::Mesh::GetIndicesCount() const noexcept
{
	return indicesCount;
}

Graphics::VertexFormat Graphics::Mesh::GetVertexFormat() const noexcept
{
	return vertexFormat;
}

const Graphics::BoundingBox& Graphics::Mesh::GetBoundingBox() const noexcept
{
	return boundingBox;
}

void Graphics::Mesh::Present(ID3D12GraphicsCommandList* commandList) const
{
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetIndexBuffer(&indexBufferView);
}

void Graphics::Mesh::FindBoundingBox(const void* verticesData, size_t verticesDataSize, VertexFormat _vertexFormat, BoundingBox& result)
{
	uint32_t vertexStride = GetVertexStride(_vertexFormat);

	result = { *reinterpret_cast<const float3*>(verticesData), *reinterpret_cast<const float3*>(verticesData) };

	uint32_t verticesCount = verticesDataSize / vertexStride;

	for (uint32_t vertexId = 0; vertexId < verticesCount; vertexId++)
	{
		const float3& position = *(reinterpret_cast<const float3*>(reinterpret_cast<const uint8_t*>(verticesData) + vertexId * vertexStride));

		if (result.minCornerPoint.x > position.x)
			result.minCornerPoint.x = position.x;

		if (result.minCornerPoint.y > position.y)
			result.minCornerPoint.y = position.y;

		if (result.minCornerPoint.z > position.z)
			result.minCornerPoint.z = position.z;

		if (result.maxCornerPoint.x < position.x)
			result.maxCornerPoint.x = position.x;

		if (result.maxCornerPoint.y < position.y)
			result.maxCornerPoint.y = position.y;

		if (result.maxCornerPoint.z < position.z)
			result.maxCornerPoint.z = position.z;
	}
}
