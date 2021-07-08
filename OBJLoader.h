#pragma once

#include "GraphicsHelper.h"

namespace Graphics
{
	class OBJLoader
	{
	public:
		static void Load(const std::filesystem::path& filePath, bool calculateNormals, bool calculateTangents, bool smoothNormals,
			VertexFormat& vertexFormat, std::vector<uint8_t>& verticesData, std::vector<uint8_t>& indicesData);

	private:
		OBJLoader() {};
		~OBJLoader() {};

		OBJLoader(const OBJLoader&) = delete;
		OBJLoader(OBJLoader&&) = delete;
		OBJLoader& operator=(const OBJLoader&) = delete;
		OBJLoader& operator=(OBJLoader&&) = delete;

		static std::string GetToken(const std::string& objLine);
		static float2 GetVector2(const std::string& objLine);
		static float3 GetVector3(const std::string& objLine);

		static VertexFormat GetFaceFormat(size_t texCoordCount, size_t normalCount);
		static void GetFace(const std::string& objLine, VertexFormat vertexFormat, std::vector<uint32_t>& face);

		static void ComposeVertices(VertexFormat vertexFormat, const std::vector<float3>& positions, const std::vector<float3>& normals,
			const std::vector<float3>& tangents, const std::vector<float3>& binormals, const std::vector<float2>& texCoords, const std::vector<uint32_t>& faces, 
			std::vector<float>& vertices, std::vector<uint32_t>& indices);
	};
}
