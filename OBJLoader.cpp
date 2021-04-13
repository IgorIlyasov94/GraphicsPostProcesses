#include "OBJLoader.h"

void Graphics::OBJLoader::Load(const std::filesystem::path& filePath, bool calculateNormals, VertexFormat& vertexFormat, std::vector<uint8_t>& verticesData,
	std::vector<uint8_t>& indicesData)
{
	std::ifstream objFile(filePath, std::ios::in);

	std::vector<float3> positions;
	std::vector<float3> normals;
	std::vector<float2> texCoords;

	std::vector<uint32_t> faces;
	vertexFormat = VertexFormat::UNDEFINED;

	std::string objLine;

	while (std::getline(objFile, objLine))
	{
		std::string token = GetToken(objLine);

		if (token == "v")
			positions.push_back(GetVector3(objLine));
		else if (token == "vn")
			normals.push_back(GetVector3(objLine));
		else if (token == "vt")
			texCoords.push_back(GetVector2(objLine));
		else if (token == "f")
		{
			std::vector<uint32_t> face;

			vertexFormat = GetFaceFormat(texCoords.size(), normals.size());

			GetFace(objLine, vertexFormat, face);
			TriangulateFace(vertexFormat, positions, face);

			for (auto& attributeIndex : face)
				faces.push_back(attributeIndex);
		}
	}

	if (vertexFormat == VertexFormat::UNDEFINED)
		throw std::exception("OBJLoader::Load: Wrong file format");

	if (calculateNormals)
	{
		CalculateNormals(vertexFormat, positions, faces, normals);
		SmoothNormals(vertexFormat, positions, faces, normals);

		if (vertexFormat == VertexFormat::POSITION)
			vertexFormat = VertexFormat::POSITION_NORMAL;
		else if (vertexFormat == VertexFormat::POSITION_TEXCOORD)
			vertexFormat = VertexFormat::POSITION_NORMAL_TEXCOORD;
	}

	std::vector<float> vertices;
	std::vector<uint32_t> indices;

	ComposeVertices(vertexFormat, positions, normals, texCoords, faces, vertices, indices);

	verticesData.resize(vertices.size() * sizeof(float));
	std::copy(reinterpret_cast<uint8_t*>(vertices.data()), reinterpret_cast<uint8_t*>(vertices.data()) + vertices.size() * sizeof(float), verticesData.data());

	indicesData.resize(indices.size() * sizeof(float));
	std::copy(reinterpret_cast<uint8_t*>(indices.data()), reinterpret_cast<uint8_t*>(indices.data()) + indices.size() * sizeof(uint32_t), indicesData.data());
}

std::string Graphics::OBJLoader::GetToken(const std::string& objLine)
{
	std::stringstream objLineStream(objLine);

	std::string token;

	objLineStream >> token;

	return token;
}

float2 Graphics::OBJLoader::GetVector2(const std::string& objLine)
{
	std::stringstream objLineStream(objLine);

	std::string token;
	float2 result{};

	objLineStream >> token >> result.x >> result.y;

	return result;
}

float3 Graphics::OBJLoader::GetVector3(const std::string& objLine)
{
	std::stringstream objLineStream(objLine);

	std::string token;
	float3 result{};

	objLineStream >> token >> result.x >> result.y >> result.z;

	return result;
}

Graphics::VertexFormat Graphics::OBJLoader::GetFaceFormat(size_t texCoordCount, size_t normalCount)
{
	VertexFormat faceFormat = VertexFormat::UNDEFINED;

	if (normalCount > 0)
	{
		if (texCoordCount > 0)
			faceFormat = VertexFormat::POSITION_NORMAL_TEXCOORD;
		else
			faceFormat = VertexFormat::POSITION_NORMAL;
	}
	else
		if (texCoordCount > 0)
			faceFormat = VertexFormat::POSITION_TEXCOORD;
		else
			faceFormat = VertexFormat::POSITION;

	return faceFormat;
}

void Graphics::OBJLoader::GetFace(const std::string& objLine, VertexFormat faceFormat, std::vector<uint32_t>& face)
{
	std::stringstream objLineStream(objLine);

	std::string token;
	
	objLineStream >> token;

	while (!objLineStream.eof())
	{
		uint32_t attributeIndex;

		objLineStream >> attributeIndex;
		objLineStream.ignore(1);

		if (objLineStream.bad() || objLineStream.fail())
			break;

		face.push_back(--attributeIndex);

		if (faceFormat == VertexFormat::POSITION_NORMAL)
		{
			objLineStream.ignore(1);
			objLineStream >> attributeIndex;

			face.push_back(--attributeIndex);
		}
	}
}

void Graphics::OBJLoader::TriangulateFace(VertexFormat vertexFormat, const std::vector<float3>& positions, std::vector<uint32_t>& face)
{
	uint32_t faceStride = (vertexFormat == VertexFormat::POSITION) ? 1 : (vertexFormat == VertexFormat::POSITION_NORMAL_TEXCOORD) ? 3 : 2;
	uint32_t verticesCount = face.size() / faceStride;

	if (verticesCount == 3)
		return;

	std::vector<uint32_t> newFace;

	if (verticesCount < 3)
	{
		face = newFace;
		return;
	}

	std::list<uint32_t> freeVertices;

	for (uint32_t vertexId = 0; vertexId < verticesCount; vertexId++)
		freeVertices.push_back(vertexId);

	auto vertexItPrevious = freeVertices.begin();
	auto vertexItCurrent = freeVertices.begin();
	vertexItCurrent++;
	auto vertexItNext = freeVertices.begin();
	vertexItNext++++;
	
	float3 faceNormal = CalculatePolygonNormal(vertexFormat, positions, face);

	while (freeVertices.size() > 2)
	{
		bool triangleIsIncorrect = false;

		const float3& positionPrevious = positions[face[*vertexItPrevious * faceStride]];
		const float3& positionCurrent = positions[face[*vertexItCurrent * faceStride]];
		const float3& positionNext = positions[face[*vertexItNext * faceStride]];

		for (auto& vertexId : freeVertices)
			if (vertexId != *vertexItPrevious && vertexId != *vertexItCurrent && vertexId != *vertexItNext)
				if (CheckPointInTriangle(positionPrevious, positionCurrent, positionNext, positions[face[vertexId * faceStride]]))
				{
					triangleIsIncorrect = true;

					break;
				}

		if (!triangleIsIncorrect)
			if (CheckTriangleInPolygon(positionPrevious, positionCurrent, positionNext, faceNormal))
				triangleIsIncorrect = true;

		if (!triangleIsIncorrect)
		{
			for (uint32_t faceAttributeId = 0; faceAttributeId < faceStride; faceAttributeId++)
				newFace.push_back(face[*vertexItPrevious * faceStride + faceAttributeId]);
			for (uint32_t faceAttributeId = 0; faceAttributeId < faceStride; faceAttributeId++)
				newFace.push_back(face[*vertexItCurrent * faceStride + faceAttributeId]);
			for (uint32_t faceAttributeId = 0; faceAttributeId < faceStride; faceAttributeId++)
				newFace.push_back(face[*vertexItNext * faceStride + faceAttributeId]);

			freeVertices.erase(vertexItCurrent);

			vertexItCurrent = vertexItNext;
		}
		else
		{
			vertexItPrevious++;

			if (vertexItPrevious == freeVertices.end())
				vertexItPrevious = freeVertices.begin();
		}

		vertexItCurrent++;
		vertexItNext++;

		if (vertexItCurrent == freeVertices.end())
			vertexItCurrent = freeVertices.begin();
		if (vertexItNext == freeVertices.end())
			vertexItNext = freeVertices.begin();
	}

	face = newFace;
}

float3 Graphics::OBJLoader::CalculatePolygonCenter(VertexFormat vertexFormat, const std::vector<float3>& positions, const std::vector<uint32_t>& face)
{
	uint32_t faceStride = (vertexFormat == VertexFormat::POSITION) ? 1 : (vertexFormat == VertexFormat::POSITION_NORMAL_TEXCOORD) ? 3 : 2;
	uint32_t verticesCount = face.size() / faceStride;

	floatN positionSum{};

	for (uint32_t vertexId = 0; vertexId < verticesCount; vertexId += faceStride)
	{
		positionSum += XMLoadFloat3(&positions[face[vertexId]]);
	}

	float3 result{};

	XMStoreFloat3(&result, positionSum / static_cast<float>(verticesCount));

	return result;
}

float3 Graphics::OBJLoader::CalculatePolygonNormal(VertexFormat vertexFormat, const std::vector<float3>& positions, const std::vector<uint32_t>& face)
{
	uint32_t faceStride = (vertexFormat == VertexFormat::POSITION) ? 1 : (vertexFormat == VertexFormat::POSITION_NORMAL_TEXCOORD) ? 3 : 2;
	uint32_t verticesCount = face.size() / faceStride;

	floatN normal{};

	float3 positionCenter = CalculatePolygonCenter(vertexFormat, positions, face);

	for (uint32_t vertexId = 0; vertexId < verticesCount; vertexId += faceStride)
	{
		const float3& positionCurrent = positions[face[vertexId]];
		const float3& positionNext = positions[face[(vertexId + faceStride) % verticesCount]];

		floatN vector0 = XMLoadFloat3(&positionCurrent) - XMLoadFloat3(&positionCenter);
		floatN vector1 = XMLoadFloat3(&positionNext) - XMLoadFloat3(&positionCenter);

		normal += XMVector3Cross(vector0, vector1);
	}

	normal = XMVector3NormalizeEst(normal);

	float3 result{};

	XMStoreFloat3(&result, normal);

	return result;
}

float Graphics::OBJLoader::CalculateTriangleArea(float3 position0, float3 position1, float3 position2)
{
	floatN vector0_1 = XMLoadFloat3(&position1) - XMLoadFloat3(&position0);
	floatN vector1_2 = XMLoadFloat3(&position2) - XMLoadFloat3(&position1);

	floatN triangleVectorArea = XMVector3Cross(vector0_1, vector1_2);
	
	float result{};

	XMStoreFloat(&result, XMVector3Dot(triangleVectorArea, triangleVectorArea));

	return std::sqrt(result);
}

float3 Graphics::OBJLoader::CalculateBarycentric(float3 position0, float3 position1, float3 position2, float3 point)
{
	float triangleArea = CalculateTriangleArea(position0, position1, position2);
	
	float3 result{};
	result.x = CalculateTriangleArea(position1, position2, point) / triangleArea;
	result.y = CalculateTriangleArea(position2, position0, point) / triangleArea;
	result.z = CalculateTriangleArea(position0, position1, point) / triangleArea;

	return result;
}

bool Graphics::OBJLoader::CheckPointInTriangle(float3 position0, float3 position1, float3 position2, float3 point)
{
	float3 barycentric = CalculateBarycentric(position0, position1, position2, point);

	if (barycentric.x + barycentric.y + barycentric.z <= 1.0f)
		return true;

	return false;
}

bool Graphics::OBJLoader::CheckTriangleInPolygon(float3 position0, float3 position1, float3 position2, float3 polygonNormal)
{
	float3 triangleNormal = CalculateNormal(position0, position1, position2);

	float dotNN{};
	XMStoreFloat(&dotNN, XMVector3Dot(XMLoadFloat3(&triangleNormal), XMLoadFloat3(&polygonNormal)));

	return dotNN > 0.0f;
}

float3 Graphics::OBJLoader::CalculateNormal(float3 position0, float3 position1, float3 position2)
{
	floatN vector0_1 = XMLoadFloat3(&position1) - XMLoadFloat3(&position0);
	floatN vector1_2 = XMLoadFloat3(&position2) - XMLoadFloat3(&position1);

	floatN normal = XMVector3Cross(vector0_1, vector1_2);
	normal = XMVector3NormalizeEst(normal);

	float3 result;

	XMStoreFloat3(&result, normal);

	return result;
}

void Graphics::OBJLoader::CalculateNormals(VertexFormat vertexFormat, const std::vector<float3>& positions, std::vector<uint32_t>& faces, std::vector<float3>& normals)
{
	std::vector<uint32_t> newFaces;
	std::vector<float3> newNormals;

	uint32_t faceStride = (vertexFormat == VertexFormat::POSITION) ? 1 : (vertexFormat == VertexFormat::POSITION_NORMAL_TEXCOORD) ? 3 : 2;
	uint32_t trianglesCount = faces.size() / (3 * faceStride);

	for (uint32_t triangleId = 0; triangleId < trianglesCount; triangleId++)
	{
		std::array<uint32_t, 3> positionIndex{};
		std::array<uint32_t, 3> texCoordIndex{};

		for (uint32_t localVertexId = 0; localVertexId < 3; localVertexId++)
			positionIndex[localVertexId] = faces[(triangleId * 3 + localVertexId) * faceStride];

		if (vertexFormat == VertexFormat::POSITION_TEXCOORD || vertexFormat == VertexFormat::POSITION_NORMAL_TEXCOORD)
			for (uint32_t localVertexId = 0; localVertexId < 3; localVertexId++)
				texCoordIndex[localVertexId] = faces[(triangleId * 3 + localVertexId) * faceStride + 1];

		auto& position0 = positions[positionIndex[0]];
		auto& position1 = positions[positionIndex[1]];
		auto& position2 = positions[positionIndex[2]];

		float3 normal = CalculateNormal(position0, position1, position2);

		for (uint32_t localVertexId = 0; localVertexId < 3; localVertexId++)
		{
			newFaces.push_back(positionIndex[localVertexId]);
			if (vertexFormat == VertexFormat::POSITION_TEXCOORD || vertexFormat == VertexFormat::POSITION_NORMAL_TEXCOORD)
				newFaces.push_back(texCoordIndex[localVertexId]);
			newFaces.push_back(newNormals.size());
		}

		newNormals.push_back(normal);
	}

	faces = newFaces;
	normals = newNormals;
}

void Graphics::OBJLoader::SmoothNormals(VertexFormat vertexFormat, const std::vector<float3>& positions, const std::vector<uint32_t>& faces,
	std::vector<float3>& normals)
{
	uint32_t faceStride = (vertexFormat == VertexFormat::POSITION_NORMAL_TEXCOORD) ? 3 : 2;

	std::vector<float3> newNormals(normals.begin(), normals.end());

	for (uint32_t faceId = 0; faceId < faces.size(); faceId += faceStride)
	{
		std::vector<uint32_t> samePositionFaceIndices = GetFaceIndicesForSamePosition(vertexFormat, positions, faces, 0, positions[faces[faceId]]);

		floatN averageNormal{};

		for (auto& samePositionFaceIndex : samePositionFaceIndices)
			averageNormal += XMLoadFloat3(&normals[faces[samePositionFaceIndex + 2]]);

		averageNormal /= static_cast<float>(samePositionFaceIndices.size());
		averageNormal = XMVector3NormalizeEst(averageNormal);

		for (auto& samePositionFaceIndex : samePositionFaceIndices)
			XMStoreFloat3(&newNormals[faces[samePositionFaceIndex + 2]], averageNormal);
	}

	normals = newNormals;
}

void Graphics::OBJLoader::ComposeVertices(VertexFormat vertexFormat, const std::vector<float3>& positions, const std::vector<float3>& normals,
	const std::vector<float2>& texCoords, const std::vector<uint32_t>& faces, std::vector<float>& vertices, std::vector<uint32_t>& indices)
{
	auto facesIterator = faces.begin();
	uint32_t vertex4ByteStride = GetVertex4ByteStride(vertexFormat);

	while (facesIterator != faces.end())
	{
		float3 position = positions[*facesIterator++];

		float3 normal{};
		float2 texCoord{};

		if (vertexFormat == VertexFormat::POSITION_TEXCOORD || vertexFormat == VertexFormat::POSITION_NORMAL_TEXCOORD)
			texCoord = texCoords[*facesIterator++];

		if (vertexFormat == VertexFormat::POSITION_NORMAL || vertexFormat == VertexFormat::POSITION_NORMAL_TEXCOORD)
			normal = normals[*facesIterator++];

		int64_t newIndex = GetIndexForSameVertex(vertex4ByteStride, vertices, indices, position, normal, texCoord);

		if (newIndex == -1)
		{
			vertices.push_back(position.x);
			vertices.push_back(position.y);
			vertices.push_back(position.z);

			if (vertexFormat == VertexFormat::POSITION_NORMAL || vertexFormat == VertexFormat::POSITION_NORMAL_TEXCOORD)
			{
				vertices.push_back(normal.x);
				vertices.push_back(normal.y);
				vertices.push_back(normal.z);
			}

			if (vertexFormat == VertexFormat::POSITION_TEXCOORD || vertexFormat == VertexFormat::POSITION_NORMAL_TEXCOORD)
			{
				vertices.push_back(texCoord.x);
				vertices.push_back(texCoord.y);
			}

			indices.push_back(vertices.size() / vertex4ByteStride - 1);
		}
		else
		{
			indices.push_back(static_cast<uint32_t>(newIndex));
		}
	}
}

const std::vector<uint32_t> Graphics::OBJLoader::GetFaceIndicesForSamePosition(VertexFormat vertexFormat, const std::vector<float3>& positions,
	const std::vector<uint32_t>& faces, uint32_t startFaceIndex, float3 position)
{
	const float epsilon = 0.00001f;

	std::vector<uint32_t> positionIndices;

	uint32_t faceStride = (vertexFormat == VertexFormat::POSITION) ? 1 : (vertexFormat == VertexFormat::POSITION_NORMAL_TEXCOORD) ? 3 : 2;

	for (uint32_t faceId = startFaceIndex; faceId < faces.size(); faceId += faceStride)
	{
		if (std::abs(positions[faces[faceId]].x - position.x) > epsilon)
			continue;

		if (std::abs(positions[faces[faceId]].y - position.y) > epsilon)
			continue;

		if (std::abs(positions[faces[faceId]].z - position.z) > epsilon)
			continue;

		positionIndices.push_back(faceId);
	}

	return positionIndices;
}

int64_t Graphics::OBJLoader::GetIndexForSameVertex(uint32_t vertex4ByteStride, const std::vector<float>& vertices, const std::vector<uint32_t>& indices,
	float3 position, float3 normal, float2 texCoord)
{
	const float epsilon = 0.00001f;

	for (auto& vertexIndex : indices)
	{
		uint32_t stridedVertexIndex = vertexIndex * vertex4ByteStride;

		if (std::abs(vertices[stridedVertexIndex] - position.x) > epsilon)
			continue;

		if (std::abs(vertices[stridedVertexIndex + 1] - position.y) > epsilon)
			continue;

		if (std::abs(vertices[stridedVertexIndex + 2] - position.z) > epsilon)
			continue;

		if (vertex4ByteStride == 5)
		{
			if (std::abs(vertices[stridedVertexIndex + 3] - texCoord.x) > epsilon)
				continue;

			if (std::abs(vertices[stridedVertexIndex + 4] - texCoord.y) > epsilon)
				continue;
		}
		else if (vertex4ByteStride > 5)
		{
			if (std::abs(vertices[stridedVertexIndex + 3] - normal.x) > epsilon)
				continue;

			if (std::abs(vertices[stridedVertexIndex + 4] - normal.y) > epsilon)
				continue;

			if (std::abs(vertices[stridedVertexIndex + 5] - normal.z) > epsilon)
				continue;

			if (vertex4ByteStride == 8)
			{
				if (std::abs(vertices[stridedVertexIndex + 6] - texCoord.x) > epsilon)
					continue;

				if (std::abs(vertices[stridedVertexIndex + 7] - texCoord.y) > epsilon)
					continue;
			}
		}

		return vertexIndex;
	}

	return -1;
}

uint32_t Graphics::OBJLoader::GetVertex4ByteStride(VertexFormat vertexFormat)
{
	if (vertexFormat == VertexFormat::POSITION_TEXCOORD)
		return 5;
	else if (vertexFormat == VertexFormat::POSITION_NORMAL)
		return 6;
	else if (vertexFormat == VertexFormat::POSITION_NORMAL_TEXCOORD)
		return 8;

	return 3;
}
