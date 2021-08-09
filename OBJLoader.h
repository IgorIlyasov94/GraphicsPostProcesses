#pragma once

#include "IMeshLoader.h"
#include "GeometryStructures.h"

namespace Graphics
{
	class OBJLoader : public IMeshLoader
	{
	public:
		OBJLoader() {};
		~OBJLoader() {};

		void Load(const std::filesystem::path& filePath, SplittedMeshData& splittedMeshData) override final;

	private:
		std::string GetToken(const std::string& objLine);
		float2 GetVector2(const std::string& objLine);
		float3 GetVector3(const std::string& objLine);

		VertexFormat GetFaceFormat(size_t texCoordCount, size_t normalCount);
		void GetFace(const std::string& objLine, VertexFormat vertexFormat, std::vector<size_t>& positionFace, std::vector<size_t>& normalFace,
			std::vector<size_t>& texCoordFace, size_t& verticesPerFace);
	};
}
