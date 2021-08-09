#pragma once

#include "GraphicsHelper.h"
#include "GeometryStructures.h"

namespace Graphics
{
	class MeshProcessor
	{
	public:
		MeshProcessor(const SplittedMeshData& splittedMeshData);
		~MeshProcessor() {};

		const SplittedMeshData& GetSplittedData() const noexcept;
		bool GetComposedData(std::vector<std::shared_ptr<Vertex>>& composedVertices, std::vector<uint32_t>& indices);
		bool GetRawComposedData(std::vector<uint32_t>& rawVertexBuffer, std::vector<uint32_t>& rawIndexBuffer);
		BoundingBox GetBoundingBox() const noexcept;

		void CalculateNormals(bool smooth);
		void CalculateTangents();
		void SmoothNormals();

		void ConvertPolygons(PolygonFormat targetPolygonFormat);

		void Compose(VertexFormat targetVertexFormat, bool enableOptimization, VertexFormat& resultVertexFormat);
		
	private:
		MeshProcessor() = delete;

		VertexFormat GetResultVertexFormat(VertexFormat targetVertexFormat);
		int64_t FindVertex(const std::vector<std::shared_ptr<Vertex>>& meshVertices, const Vertex& vertex);
		
		void AddNormalsIfRequired(VertexFormat targetVertexFormat, VertexFormat& resultVertexFormat);
		void AddTangentsIfRequired(VertexFormat targetVertexFormat, VertexFormat& resultVertexFormat);

		SplittedMeshData meshData;
		std::vector<std::shared_ptr<Vertex>> composedMeshVertices;
		std::vector<uint32_t> composedMeshIndices;
		PolygonFormat currentPolygonFormat;
	};
}
