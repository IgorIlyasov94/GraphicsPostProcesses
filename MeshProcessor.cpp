#include "MeshProcessor.h"
#include "GeometryProcessor.h"

Graphics::MeshProcessor::MeshProcessor(const SplittedMeshData& splittedMeshData)
	: meshData(splittedMeshData), currentPolygonFormat(PolygonFormat::N_GON)
{
	auto minVerticesPerFaceIt = std::min_element(meshData.verticesPerFaces.begin(), meshData.verticesPerFaces.end());
	auto maxVerticesPerFaceIt = std::max_element(meshData.verticesPerFaces.begin(), meshData.verticesPerFaces.end());

	if (meshData.positionFaces.size() != meshData.normalFaces.size() && meshData.normalFaces.size() > 0 ||
		meshData.positionFaces.size() != meshData.texCoordFaces.size() && meshData.texCoordFaces.size() > 0 || meshData.positionFaces.size() < 3 ||
		*minVerticesPerFaceIt < 3)
		throw std::exception("MeshProcessor::MeshProcessor: Incorrect mesh data");

	if (*minVerticesPerFaceIt == *maxVerticesPerFaceIt)
		if (*minVerticesPerFaceIt == 3)
			currentPolygonFormat = PolygonFormat::TRIANGLE;
		else if (*minVerticesPerFaceIt == 4)
			currentPolygonFormat = PolygonFormat::QUAD;
}

const Graphics::SplittedMeshData& Graphics::MeshProcessor::GetSplittedData() const noexcept
{
	return meshData;
}

bool Graphics::MeshProcessor::GetComposedData(std::vector<std::shared_ptr<Vertex>>& composedVertices, std::vector<uint32_t>& indices)
{
	if (composedMeshVertices.empty() || composedMeshIndices.empty())
		return false;

	composedVertices = composedMeshVertices;
	indices = composedMeshIndices;

	return true;
}

bool Graphics::MeshProcessor::GetRawComposedData(std::vector<uint32_t>& rawVertexBuffer, std::vector<uint32_t>& rawIndexBuffer)
{
	if (composedMeshVertices.empty() || composedMeshIndices.empty())
		return false;

	rawVertexBuffer.clear();
	rawVertexBuffer.reserve(composedMeshVertices.size() * composedMeshVertices[0]->GetStride() / 4);
	
	for (auto& vertexPtr : composedMeshVertices)
	{
		std::vector<uint32_t> rawVertex;
		vertexPtr->CopyToRawChunk(rawVertex);

		rawVertexBuffer.insert(rawVertexBuffer.end(), rawVertex.begin(), rawVertex.end());
	}

	rawIndexBuffer = composedMeshIndices;

	return true;
}

Graphics::BoundingBox Graphics::MeshProcessor::GetBoundingBox() const noexcept
{
	BoundingBox result = { meshData.positions[0], meshData.positions[0] };

	for (auto& position : meshData.positions)
	{
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

	return result;
}

void Graphics::MeshProcessor::CalculateNormals(bool smooth)
{
	meshData.normals.clear();
	meshData.normalFaces.clear();

	meshData.normals.reserve(meshData.verticesPerFaces.size());
	meshData.normalFaces.reserve(meshData.positionFaces.size());

	auto positionFaceItBegin = meshData.positionFaces.begin();
	auto positionFaceItEnd = meshData.positionFaces.begin();

	for (auto& verticesPerFace : meshData.verticesPerFaces)
	{
		std::advance(positionFaceItEnd, verticesPerFace);

		auto polygonNormal = GeometryProcessor::CalculatePolygonNormal(meshData.positions, positionFaceItBegin, positionFaceItEnd);

		for (size_t vertexId = 0; vertexId < verticesPerFace; vertexId++)
			meshData.normalFaces.push_back(meshData.normals.size());

		meshData.normals.push_back(polygonNormal);

		positionFaceItBegin = positionFaceItEnd;
	}

	if (smooth)
		SmoothNormals();
}

void Graphics::MeshProcessor::CalculateTangents()
{
	meshData.tangents.clear();
	meshData.binormals.clear();

	meshData.tangents.reserve(meshData.normals.size());
	meshData.binormals.reserve(meshData.normals.size());

	for (auto& normal : meshData.normals)
	{
		float3 tangent{};
		float3 binormal{};

		GeometryProcessor::CalculateTangents(normal, tangent, binormal);

		meshData.tangents.push_back(tangent);
		meshData.binormals.push_back(binormal);
	}
}

void Graphics::MeshProcessor::SmoothNormals()
{
	std::vector<float3> newNormals;
	newNormals.reserve(meshData.positions.size());

	std::vector<size_t> newNormalFaces(meshData.normalFaces.size(), 0);

	std::set<size_t> processedVertexId;

	for (size_t vertexId = 0; vertexId < meshData.normalFaces.size(); vertexId++)
	{
		if (processedVertexId.find(vertexId) != processedVertexId.end())
			continue;

		processedVertexId.insert(vertexId);

		floatN normalSum = XMLoadFloat3(&meshData.normals[meshData.normalFaces[vertexId]]);
		size_t normalCount = 1;

		auto& currentPosition = meshData.positions[meshData.positionFaces[vertexId]];

		for (size_t vertexTraversalId = vertexId + 1; vertexTraversalId < meshData.normalFaces.size(); vertexTraversalId++)
		{
			if (processedVertexId.find(vertexTraversalId) != processedVertexId.end())
				continue;

			auto& traversalPosition = meshData.positions[meshData.positionFaces[vertexTraversalId]];

			if (XMVector3Equal(XMLoadFloat3(&currentPosition), XMLoadFloat3(&traversalPosition)))
			{
				normalSum += XMLoadFloat3(&meshData.normals[meshData.normalFaces[vertexTraversalId]]);
				normalCount++;

				newNormalFaces[vertexTraversalId] = newNormals.size();

				processedVertexId.insert(vertexTraversalId);
			}
		}

		float3 newNormal{};
		XMStoreFloat3(&newNormal, normalSum / static_cast<float>(normalCount));

		newNormalFaces[vertexId] = newNormals.size();
		newNormals.push_back(newNormal);
	}

	newNormals.shrink_to_fit();
	meshData.normals = newNormals;
	meshData.normalFaces = newNormalFaces;
}

void Graphics::MeshProcessor::ConvertPolygons(PolygonFormat targetPolygonFormat)
{
	SplittedMeshData newMeshData;
	newMeshData.positionFaces.reserve(meshData.positionFaces.size());
	newMeshData.normalFaces.reserve(meshData.normalFaces.size());
	newMeshData.texCoordFaces.reserve(meshData.texCoordFaces.size());

	auto positionFaceItBegin = meshData.positionFaces.begin();
	auto positionFaceItEnd = meshData.positionFaces.begin();

	for (auto& verticesPerFace : meshData.verticesPerFaces)
	{
		auto vertexStartFaceIndex = std::distance(meshData.positionFaces.begin(), positionFaceItBegin);
		std::advance(positionFaceItEnd, verticesPerFace);

		std::vector<size_t> relativeVertexIndices;
		GeometryProcessor::ConvertPolygon(targetPolygonFormat, meshData.positions, positionFaceItBegin, positionFaceItEnd, relativeVertexIndices);

		for (auto& relativeVertexIndex : relativeVertexIndices)
		{
			newMeshData.positionFaces.push_back(meshData.positionFaces[vertexStartFaceIndex + relativeVertexIndex]);
			newMeshData.normalFaces.push_back(meshData.normalFaces[vertexStartFaceIndex + relativeVertexIndex]);
			newMeshData.texCoordFaces.push_back(meshData.texCoordFaces[vertexStartFaceIndex + relativeVertexIndex]);
		}

		positionFaceItBegin = positionFaceItEnd;
	}

	meshData.positionFaces = newMeshData.positionFaces;
	meshData.normalFaces = newMeshData.normalFaces;
	meshData.texCoordFaces = newMeshData.texCoordFaces;
}

void Graphics::MeshProcessor::Compose(VertexFormat targetVertexFormat, bool enableOptimization, VertexFormat& resultVertexFormat)
{
	composedMeshVertices.clear();
	composedMeshIndices.clear();
	composedMeshVertices.reserve(meshData.positionFaces.size());
	composedMeshIndices.reserve(meshData.positionFaces.size());

	resultVertexFormat = GetResultVertexFormat(targetVertexFormat);

	AddNormalsIfRequired(targetVertexFormat, resultVertexFormat);
	AddTangentsIfRequired(targetVertexFormat, resultVertexFormat);

	for (size_t vertexIndexId = 0; vertexIndexId < meshData.positionFaces.size(); vertexIndexId++)
	{
		auto currentVertex = std::make_shared<Vertex>();
		currentVertex->format = resultVertexFormat;

		currentVertex->position = meshData.positions[meshData.positionFaces[vertexIndexId]];
		
		if ((resultVertexFormat & VertexFormat::NORMAL) == VertexFormat::NORMAL)
			currentVertex->normal = meshData.normals[meshData.normalFaces[vertexIndexId]];

		if ((resultVertexFormat & VertexFormat::TANGENT_BINORMAL) == VertexFormat::TANGENT_BINORMAL)
		{
			currentVertex->tangent = meshData.tangents[meshData.normalFaces[vertexIndexId]];
			currentVertex->binormal = meshData.binormals[meshData.normalFaces[vertexIndexId]];
		}

		if ((resultVertexFormat & VertexFormat::TEXCOORD) == VertexFormat::TEXCOORD)
			currentVertex->texCoord = meshData.texCoords[meshData.texCoordFaces[vertexIndexId]];

		int64_t newIndex = -1;

		if (enableOptimization)
			newIndex = FindVertex(composedMeshVertices, *currentVertex);

		if (newIndex == -1)
		{
			composedMeshIndices.push_back(static_cast<uint32_t>(composedMeshVertices.size()));
			composedMeshVertices.push_back(currentVertex);
		}
		else
			composedMeshIndices.push_back(static_cast<uint32_t>(newIndex));
	}

	composedMeshVertices.shrink_to_fit();
	composedMeshIndices.shrink_to_fit();
}

Graphics::VertexFormat Graphics::MeshProcessor::GetResultVertexFormat(VertexFormat targetVertexFormat)
{
	VertexFormat resultVertexFormat = targetVertexFormat;

	if (resultVertexFormat != VertexFormat::UNDEFINED)
	{
		if (meshData.positions.empty() || meshData.positionFaces.empty())
			resultVertexFormat &= ~VertexFormat::POSITION;

		if (meshData.normals.empty() || meshData.normalFaces.empty())
			resultVertexFormat &= ~VertexFormat::NORMAL;

		if (meshData.tangents.empty() || meshData.binormals.empty() || meshData.normalFaces.empty())
			resultVertexFormat &= ~VertexFormat::TANGENT_BINORMAL;

		if (meshData.texCoords.empty() || meshData.texCoordFaces.empty())
			resultVertexFormat &= ~VertexFormat::TEXCOORD;
	}

	return resultVertexFormat;
}

int64_t Graphics::MeshProcessor::FindVertex(const std::vector<std::shared_ptr<Vertex>>& meshVertices, const Vertex& vertex)
{
	for (size_t vertexId = 0; vertexId < meshVertices.size(); vertexId++)
		if (*meshVertices[vertexId] == vertex)
			return vertexId;

	return -1;
}

void Graphics::MeshProcessor::AddNormalsIfRequired(VertexFormat targetVertexFormat, VertexFormat& resultVertexFormat)
{
	if ((targetVertexFormat & VertexFormat::NORMAL) == VertexFormat::NORMAL &&
		(resultVertexFormat & VertexFormat::NORMAL) != VertexFormat::NORMAL)
	{
		CalculateNormals(false);

		resultVertexFormat |= VertexFormat::NORMAL;
	}
}

void Graphics::MeshProcessor::AddTangentsIfRequired(VertexFormat targetVertexFormat, VertexFormat& resultVertexFormat)
{
	if ((targetVertexFormat & VertexFormat::TANGENT_BINORMAL) == VertexFormat::TANGENT_BINORMAL &&
		(resultVertexFormat & VertexFormat::TANGENT_BINORMAL) != VertexFormat::TANGENT_BINORMAL)
	{
		if ((targetVertexFormat & VertexFormat::NORMAL) != VertexFormat::NORMAL)
			targetVertexFormat |= VertexFormat::NORMAL;

		AddNormalsIfRequired(targetVertexFormat, resultVertexFormat);

		CalculateTangents();

		resultVertexFormat |= VertexFormat::TANGENT_BINORMAL;
	}
}
