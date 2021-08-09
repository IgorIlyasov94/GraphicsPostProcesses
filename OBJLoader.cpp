#include "OBJLoader.h"

void Graphics::OBJLoader::Load(const std::filesystem::path& filePath, SplittedMeshData& splittedMeshData)
{
	std::ifstream objFile(filePath, std::ios::in);

	splittedMeshData.Clear();

	std::string objLine;

	while (std::getline(objFile, objLine))
	{
		std::string token = GetToken(objLine);

		if (token == "v")
			splittedMeshData.positions.push_back(GetVector3(objLine));
		else if (token == "vn")
			splittedMeshData.normals.push_back(GetVector3(objLine));
		else if (token == "vt")
			splittedMeshData.texCoords.push_back(GetVector2(objLine));
		else if (token == "f")
		{
			auto vertexFormat = GetFaceFormat(splittedMeshData.texCoords.size(), splittedMeshData.normals.size());

			std::vector<size_t> positionFace;
			std::vector<size_t> normalFace;
			std::vector<size_t> texCoordFace;
			size_t verticesPerFace;

			GetFace(objLine, vertexFormat, positionFace, normalFace, texCoordFace, verticesPerFace);

			splittedMeshData.positionFaces.insert(splittedMeshData.positionFaces.end(), positionFace.begin(), positionFace.end());
			splittedMeshData.normalFaces.insert(splittedMeshData.normalFaces.end(), normalFace.begin(), normalFace.end());
			splittedMeshData.texCoordFaces.insert(splittedMeshData.texCoordFaces.end(), texCoordFace.begin(), texCoordFace.end());
			splittedMeshData.verticesPerFaces.push_back(verticesPerFace);
		}
	}

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
	VertexFormat faceFormat = VertexFormat::POSITION;

	if (normalCount > 0)
		faceFormat |= VertexFormat::NORMAL;

	if (texCoordCount > 0)
		faceFormat |= VertexFormat::TEXCOORD;

	return faceFormat;
}

void Graphics::OBJLoader::GetFace(const std::string& objLine, VertexFormat vertexFormat, std::vector<size_t>& positionFace, std::vector<size_t>& normalFace,
	std::vector<size_t>& texCoordFace, size_t& verticesPerFace)
{
	verticesPerFace = 0;

	std::stringstream objLineStream(objLine);

	std::string token;

	objLineStream >> token;

	while (!objLineStream.eof())
	{
		size_t attributeIndex;

		objLineStream >> attributeIndex;

		if (objLineStream.bad() || objLineStream.fail())
			break;

		positionFace.push_back(--attributeIndex);

		verticesPerFace++;

		if (vertexFormat == VertexFormat::POSITION)
			continue;

		objLineStream.ignore((vertexFormat == (VertexFormat::POSITION | VertexFormat::NORMAL)) ? 2 : 1);
		objLineStream >> attributeIndex;

		if (vertexFormat == (VertexFormat::POSITION | VertexFormat::NORMAL))
			normalFace.push_back(--attributeIndex);
		else
			texCoordFace.push_back(--attributeIndex);

		if (vertexFormat == (VertexFormat::POSITION | VertexFormat::NORMAL | VertexFormat::TEXCOORD))
		{
			objLineStream.ignore(1);

			objLineStream >> attributeIndex;
			normalFace.push_back(--attributeIndex);
		}
	}
}
