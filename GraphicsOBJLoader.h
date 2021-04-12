#pragma once

#include "GraphicsHelper.h"

class GraphicsOBJLoader
{
public:
	static void Load(const std::filesystem::path& filePath, bool calculateNormals, VertexFormat& vertexFormat, std::vector<uint8_t>& verticesData,
		std::vector<uint8_t>& indicesData);

private:
	GraphicsOBJLoader() {};
	~GraphicsOBJLoader() {};

	GraphicsOBJLoader(const GraphicsOBJLoader&) = delete;
	GraphicsOBJLoader(GraphicsOBJLoader&&) = delete;
	GraphicsOBJLoader& operator=(const GraphicsOBJLoader&) = delete;
	GraphicsOBJLoader& operator=(GraphicsOBJLoader&&) = delete;

	static std::string GetToken(const std::string& objLine);
	static float2 GetVector2(const std::string& objLine);
	static float3 GetVector3(const std::string& objLine);

	static VertexFormat GetFaceFormat(size_t texCoordCount, size_t normalCount);
	static void GetFace(const std::string& objLine, VertexFormat vertexFormat, std::vector<uint32_t>& face);

	static void TriangulateFace(VertexFormat vertexFormat, const std::vector<float3>& positions, std::vector<uint32_t>& face);
	static float3 CalculatePolygonCenter(VertexFormat vertexFormat, const std::vector<float3>& positions, const std::vector<uint32_t>& face);
	static float3 CalculatePolygonNormal(VertexFormat vertexFormat, const std::vector<float3>& positions, const std::vector<uint32_t>& face);
	static float CalculateTriangleArea(float3 position0, float3 position1, float3 position2);
	static float3 CalculateBarycentric(float3 position0, float3 position1, float3 position2, float3 point);
	static bool CheckPointInTriangle(float3 position0, float3 position1, float3 position2, float3 point);
	static bool CheckTriangleInPolygon(float3 position0, float3 position1, float3 position2, float3 polygonNormal);
	
	static float3 CalculateNormal(float3 position0, float3 position1, float3 position2);
	static void CalculateNormals(VertexFormat vertexFormat, const std::vector<float3>& positions, std::vector<uint32_t>& faces, std::vector<float3>& normals);
	static void SmoothNormals(VertexFormat vertexFormat, const std::vector<float3>& positions, const std::vector<uint32_t>& faces, std::vector<float3>& normals);
	
	static void ComposeVertices(VertexFormat vertexFormat, const std::vector<float3>& positions, const std::vector<float3>& normals,
		const std::vector<float2>& texCoords, const std::vector<uint32_t>& faces, std::vector<float>& vertices, std::vector<uint32_t>& indices);

	static const std::vector<uint32_t> GetFaceIndicesForSamePosition(VertexFormat vertexFormat, const std::vector<float3>& positions,
		const std::vector<uint32_t>& faces, uint32_t startFaceIndex, float3 position);

	static int64_t GetIndexForSameVertex(uint32_t vertex4ByteStride, const std::vector<float>& vertices, const std::vector<uint32_t>& indices,
		float3 position, float3 normal, float2 texCoord);

	static uint32_t GetVertex4ByteStride(VertexFormat vertexFormat);
};
