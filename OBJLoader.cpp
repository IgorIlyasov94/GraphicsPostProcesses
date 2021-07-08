#include "OBJLoader.h"

void Graphics::OBJLoader::Load(const std::filesystem::path& filePath, bool calculateNormals, bool calculateTangents, bool smoothNormals,
	VertexFormat& vertexFormat, std::vector<uint8_t>& verticesData, std::vector<uint8_t>& indicesData)
{
	std::ifstream objFile(filePath, std::ios::in);

	std::vector<float3> positions;
	std::vector<float3> normals;
	std::vector<float3> tangents;
	std::vector<float3> binormals;
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

		if (vertexFormat == VertexFormat::POSITION)
			vertexFormat = VertexFormat::POSITION_NORMAL;
		else if (vertexFormat == VertexFormat::POSITION_TEXCOORD)
			vertexFormat = VertexFormat::POSITION_NORMAL_TEXCOORD;
	}

	if (smoothNormals)
		if (vertexFormat == VertexFormat::POSITION_NORMAL || vertexFormat == VertexFormat::POSITION_NORMAL_TEXCOORD)
			SmoothNormals(vertexFormat, positions, faces, normals);

	if (calculateTangents && vertexFormat != VertexFormat::POSITION && vertexFormat != VertexFormat::POSITION_TEXCOORD)
	{
		CalculateTangents(normals, tangents, binormals);

		if (vertexFormat == VertexFormat::POSITION_NORMAL)
			vertexFormat = VertexFormat::POSITION_NORMAL_TANGENT_BINORMAL;
		else if (vertexFormat == VertexFormat::POSITION_NORMAL_TEXCOORD)
			vertexFormat = VertexFormat::POSITION_NORMAL_TANGENT_BINORMAL_TEXCOORD;
	}

	std::vector<float> vertices;
	std::vector<uint32_t> indices;

	ComposeVertices(vertexFormat, positions, normals, tangents, binormals, texCoords, faces, vertices, indices);

	verticesData.resize(vertices.size() * sizeof(float));
	std::copy(reinterpret_cast<uint8_t*>(vertices.data()), reinterpret_cast<uint8_t*>(vertices.data()) + vertices.size() * sizeof(float), verticesData.data());

	indicesData.resize(indices.size() * sizeof(uint32_t));
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

		if (objLineStream.bad() || objLineStream.fail())
			break;

		face.push_back(--attributeIndex);

		if (faceFormat == VertexFormat::POSITION)
			continue;

		objLineStream.ignore((faceFormat == VertexFormat::POSITION_NORMAL) ? 2 : 1);

		objLineStream >> attributeIndex;
		face.push_back(--attributeIndex);

		if (faceFormat == VertexFormat::POSITION_NORMAL_TEXCOORD)
		{
			objLineStream.ignore(1);

			objLineStream >> attributeIndex;
			face.push_back(--attributeIndex);
		}
	}
}

void Graphics::OBJLoader::ComposeVertices(VertexFormat vertexFormat, const std::vector<float3>& positions, const std::vector<float3>& normals,
	const std::vector<float3>& tangents, const std::vector<float3>& binormals, const std::vector<float2>& texCoords, const std::vector<uint32_t>& faces,
	std::vector<float>& vertices, std::vector<uint32_t>& indices)
{
	auto facesIterator = faces.begin();
	uint32_t vertex4ByteStride = GetVertex4ByteStride(vertexFormat);

	while (facesIterator != faces.end())
	{
		float3 position = positions[*facesIterator++];

		float3 normal{};
		float3 tangent{};
		float3 binormal{};
		float2 texCoord{};

		if (vertexFormat == VertexFormat::POSITION_TEXCOORD || vertexFormat == VertexFormat::POSITION_NORMAL_TEXCOORD ||
			vertexFormat == VertexFormat::POSITION_NORMAL_TANGENT_BINORMAL_TEXCOORD)
			texCoord = texCoords[*facesIterator++];

		uint32_t currentNormalId{};

		if (vertexFormat == VertexFormat::POSITION_NORMAL || vertexFormat == VertexFormat::POSITION_NORMAL_TEXCOORD ||
			vertexFormat == VertexFormat::POSITION_NORMAL_TANGENT_BINORMAL || vertexFormat == VertexFormat::POSITION_NORMAL_TANGENT_BINORMAL_TEXCOORD)
		{
			currentNormalId = *facesIterator++;
			normal = normals[currentNormalId];
		}

		if (vertexFormat == VertexFormat::POSITION_NORMAL_TANGENT_BINORMAL || vertexFormat == VertexFormat::POSITION_NORMAL_TANGENT_BINORMAL_TEXCOORD)
		{
			tangent = tangents[currentNormalId];
			binormal = binormals[currentNormalId];
		}

		int64_t newIndex = GetIndexForSameVertex(vertex4ByteStride, vertices, indices, position, normal, texCoord);

		if (newIndex == -1)
		{
			vertices.push_back(position.x);
			vertices.push_back(position.y);
			vertices.push_back(position.z);

			if (vertexFormat == VertexFormat::POSITION_NORMAL || vertexFormat == VertexFormat::POSITION_NORMAL_TEXCOORD ||
				vertexFormat == VertexFormat::POSITION_NORMAL_TANGENT_BINORMAL || vertexFormat == VertexFormat::POSITION_NORMAL_TANGENT_BINORMAL_TEXCOORD)
			{
				vertices.push_back(normal.x);
				vertices.push_back(normal.y);
				vertices.push_back(normal.z);
			}

			if (vertexFormat == VertexFormat::POSITION_NORMAL_TANGENT_BINORMAL || vertexFormat == VertexFormat::POSITION_NORMAL_TANGENT_BINORMAL_TEXCOORD)
			{
				vertices.push_back(tangent.x);
				vertices.push_back(tangent.y);
				vertices.push_back(tangent.z);
				vertices.push_back(binormal.x);
				vertices.push_back(binormal.y);
				vertices.push_back(binormal.z);
			}

			if (vertexFormat == VertexFormat::POSITION_TEXCOORD || vertexFormat == VertexFormat::POSITION_NORMAL_TEXCOORD ||
				vertexFormat == VertexFormat::POSITION_NORMAL_TANGENT_BINORMAL_TEXCOORD)
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
