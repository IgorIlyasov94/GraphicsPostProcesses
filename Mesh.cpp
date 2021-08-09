#include "Mesh.h"
#include "GeometryProcessor.h"
#include "MeshProcessor.h"
#include "IMeshLoader.h"
#include "OBJLoader.h"

Graphics::Mesh::Mesh(std::filesystem::path filePath, VertexFormat targetVertexFormat, bool smoothNormals, bool enableOptimization)
	: indicesCount(0), vertexFormat(VertexFormat::UNDEFINED), boundingBox{}, vertexBufferView{}, indexBufferView{}
{
	std::unique_ptr<IMeshLoader> meshLoader;

	if (filePath.extension() == ".obj" || filePath.extension() == ".OBJ")
		meshLoader = std::make_unique<Graphics::OBJLoader>();
	else
		throw std::exception("Mesh::Mesh: Unsupported file extension");

	SplittedMeshData splittedMeshData;
	meshLoader->Load(filePath, splittedMeshData);

	MeshProcessor meshProcessor(splittedMeshData);

	if (smoothNormals)
	{
		if ((targetVertexFormat & VertexFormat::NORMAL) == VertexFormat::NORMAL && (splittedMeshData.normals.empty() || splittedMeshData.normalFaces.empty()))
			meshProcessor.CalculateNormals(smoothNormals);
		else if (!splittedMeshData.normals.empty() && !splittedMeshData.normalFaces.empty())
			meshProcessor.SmoothNormals();
	}

	meshProcessor.ConvertPolygons(PolygonFormat::TRIANGLE);
	meshProcessor.Compose(targetVertexFormat, enableOptimization, vertexFormat);

	boundingBox = meshProcessor.GetBoundingBox();

	std::vector<uint32_t> verticesData;
	std::vector<uint32_t> indicesData;

	meshProcessor.GetRawComposedData(verticesData, indicesData);

	auto vertexStride = VertexStride(vertexFormat);
	vertexBufferId = resourceManager.CreateVertexBuffer(verticesData.data(), verticesData.size() * sizeof(uint32_t), vertexStride);
	indexBufferId = resourceManager.CreateIndexBuffer(indicesData.data(), indicesData.size() * sizeof(uint32_t), 4);

	indicesCount = resourceManager.GetIndexBuffer(indexBufferId).indicesCount;

	vertexBufferView = resourceManager.GetVertexBufferView(vertexBufferId);
	indexBufferView = resourceManager.GetIndexBufferView(indexBufferId);
}

Graphics::Mesh::Mesh(VertexFormat _vertexFormat, const void* verticesData, size_t verticesDataSize, const void* indicesData, size_t indicesDataSize)
	: indicesCount(0), vertexFormat(_vertexFormat), boundingBox{}, vertexBufferView{}, indexBufferView{}
{
	CalculateBoundingBox(verticesData, verticesDataSize, vertexFormat, boundingBox);

	auto vertexStride = VertexStride(vertexFormat);
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

Graphics::VertexBufferId Graphics::Mesh::GetVertexBufferId() const noexcept
{
	return vertexBufferId;
}

Graphics::IndexBufferId Graphics::Mesh::GetIndexBufferId() const noexcept
{
	return indexBufferId;
}

const Graphics::BoundingBox& Graphics::Mesh::GetBoundingBox() const noexcept
{
	return boundingBox;
}

void Graphics::Mesh::Update(ID3D12GraphicsCommandList* commandList) const
{

}

void Graphics::Mesh::Draw(ID3D12GraphicsCommandList* commandList, const Material* material) const
{
	if (material != nullptr)
		if (material->IsComposed())
		{
			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
			commandList->IASetIndexBuffer(&indexBufferView);

			material->Present(commandList);

			commandList->DrawIndexedInstanced(indicesCount, 1, 0, 0, 0);
		}
}

void Graphics::Mesh::CalculateBoundingBox(const void* verticesData, size_t verticesDataSize, VertexFormat _vertexFormat, BoundingBox& result)
{
	auto vertexStride = VertexStride(_vertexFormat);

	result = { *reinterpret_cast<const float3*>(verticesData), *reinterpret_cast<const float3*>(verticesData) };

	size_t verticesCount = verticesDataSize / vertexStride;

	for (size_t vertexId = 0; vertexId < verticesCount; vertexId++)
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
