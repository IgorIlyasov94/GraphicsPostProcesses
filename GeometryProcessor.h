#pragma once

#include "GeometryStructures.h"
#include "RingBuffer.h"

namespace Graphics
{
	class GeometryProcessor
	{
	public:
		GeometryProcessor() {};
		~GeometryProcessor() {};

		static float CalculateTriangleArea(float3 position0, float3 position1, float3 position2);
		
		static float3 CalculateNormal(float3 position0, float3 position1, float3 position2);
		static void CalculateTangents(float3 normal, float3& tangent, float3& binormal);
		static float3 CalculateBarycentric(float3 position0, float3 position1, float3 position2, float3 point);
		
		static bool CheckPointInTriangle(float3 position0, float3 position1, float3 position2, float3 point);
		static bool CheckTriangleInPolygon(float3 position0, float3 position1, float3 position2, float3 polygonNormal);

		static float3 CalculatePolygonCenter(const std::vector<float3>& positions, std::vector<size_t>::const_iterator positionFaceBegin,
			std::vector<size_t>::const_iterator positionFaceEnd);
		static float3 CalculatePolygonNormal(const std::vector<float3>& positions, std::vector<size_t>::const_iterator positionFaceBegin,
			std::vector<size_t>::const_iterator positionFaceEnd);
		static void ConvertPolygon(PolygonFormat targetPolygonFormat, const std::vector<float3>& positions, std::vector<size_t>::const_iterator positionFacesBegin,
			std::vector<size_t>::const_iterator positionFaceEnd, std::vector<size_t>& newFaceIndices);

	private:
		static void RemoveRedundantVertices(const std::vector<float3>& positions, size_t verticesPerFace, RingBufferVector<size_t>& vertexIndices,
			RingBufferVector<size_t>& vertexIndexIds);
	};
}
