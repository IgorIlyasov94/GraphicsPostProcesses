#include "GeometryProcessor.h"

float Graphics::GeometryProcessor::CalculateTriangleArea(float3 position0, float3 position1, float3 position2)
{
	floatN vector0_1 = XMLoadFloat3(&position1) - XMLoadFloat3(&position0);
	floatN vector1_2 = XMLoadFloat3(&position2) - XMLoadFloat3(&position1);

	floatN triangleVectorArea = XMVector3Cross(vector0_1, vector1_2);

	float result{};

	XMStoreFloat(&result, XMVector3Dot(triangleVectorArea, triangleVectorArea));

	return std::sqrt(result);
}

float3 Graphics::GeometryProcessor::CalculateNormal(float3 position0, float3 position1, float3 position2)
{
	floatN vector0_1 = XMLoadFloat3(&position1) - XMLoadFloat3(&position0);
	floatN vector1_2 = XMLoadFloat3(&position2) - XMLoadFloat3(&position0);

	floatN normal = XMVector3Cross(vector0_1, vector1_2);
	normal = XMVector3Normalize(normal);

	float3 result;

	XMStoreFloat3(&result, normal);

	return result;
}

void Graphics::GeometryProcessor::CalculateTangents(float3 normal, float3& tangent, float3& binormal)
{
	float3 v0(0.0f, 0.0f, 1.0f);
	float3 v1(0.0f, 1.0f, 0.0f);

	floatN t0 = XMVector3Cross(XMLoadFloat3(&normal), XMLoadFloat3(&v0));
	floatN t1 = XMVector3Cross(XMLoadFloat3(&normal), XMLoadFloat3(&v1));

	if (XMVector3Length(t0).m128_f32[0] > XMVector3Length(t1).m128_f32[0])
		XMStoreFloat3(&tangent, XMVector3Normalize(t0));
	else
		XMStoreFloat3(&tangent, XMVector3Normalize(t1));

	floatN rawBinormal = XMVector3Cross(XMLoadFloat3(&tangent), XMLoadFloat3(&normal));
	XMStoreFloat3(&binormal, XMVector3Normalize(rawBinormal));
}

float3 Graphics::GeometryProcessor::CalculateBarycentric(float3 position0, float3 position1, float3 position2, float3 point)
{
	float triangleArea = GeometryProcessor::CalculateTriangleArea(position0, position1, position2);

	float3 result{};
	result.x = GeometryProcessor::CalculateTriangleArea(position1, position2, point) / triangleArea;
	result.y = GeometryProcessor::CalculateTriangleArea(position2, position0, point) / triangleArea;
	result.z = GeometryProcessor::CalculateTriangleArea(position0, position1, point) / triangleArea;

	return result;
}

bool Graphics::GeometryProcessor::CheckPointInTriangle(float3 position0, float3 position1, float3 position2, float3 point)
{
	float3 barycentric = GeometryProcessor::CalculateBarycentric(position0, position1, position2, point);

	if (barycentric.x + barycentric.y + barycentric.z <= 1.0f)
		return true;

	return false;
}

bool Graphics::GeometryProcessor::CheckTriangleInPolygon(float3 position0, float3 position1, float3 position2, float3 polygonNormal)
{
	float3 triangleNormal = GeometryProcessor::CalculateNormal(position0, position1, position2);

	float dotNN{};
	XMStoreFloat(&dotNN, XMVector3Dot(XMLoadFloat3(&triangleNormal), XMLoadFloat3(&polygonNormal)));

	return dotNN > 0.0f;
}

float3 Graphics::GeometryProcessor::CalculatePolygonCenter(const std::vector<float3>& positions, std::vector<size_t>::const_iterator positionFaceBegin,
	std::vector<size_t>::const_iterator positionFaceEnd)
{
	size_t verticesCount = std::distance(positionFaceBegin, positionFaceEnd);

	floatN positionSum{};

	for (auto& positionIndex = positionFaceBegin; positionIndex != positionFaceEnd; positionIndex++)
		positionSum += XMLoadFloat3(&positions[*positionIndex]);

	float3 result{};
	XMStoreFloat3(&result, positionSum / static_cast<float>(verticesCount));

	return result;
}

float3 Graphics::GeometryProcessor::CalculatePolygonNormal(const std::vector<float3>& positions, std::vector<size_t>::const_iterator positionFaceBegin,
	std::vector<size_t>::const_iterator positionFaceEnd)
{
	size_t verticesCount = std::distance(positionFaceBegin, positionFaceEnd);
	float3 positionCenter = CalculatePolygonCenter(positions, positionFaceBegin, positionFaceEnd);

	floatN normal{};

	for (auto& positionIndex = positionFaceBegin; positionIndex != positionFaceEnd; positionIndex++)
	{
		std::vector<size_t>::const_iterator nextPositionIndex = positionIndex;
		if (++nextPositionIndex == positionFaceEnd)
			nextPositionIndex = positionFaceBegin;

		const float3& positionCurrent = positions[*positionIndex];
		const float3& positionNext = positions[*nextPositionIndex];

		floatN vector0 = XMLoadFloat3(&positionCurrent) - XMLoadFloat3(&positionCenter);
		floatN vector1 = XMLoadFloat3(&positionNext) - XMLoadFloat3(&positionCenter);

		normal += XMVector3Cross(vector0, vector1);
	}

	if (XMVector3Length(normal).m128_f32[0] <= 0.0f)
		normal = { 0.0f, 1.0f, 0.0f, 0.0f };

	normal = XMVector3Normalize(normal);

	float3 result{};
	XMStoreFloat3(&result, normal);

	return result;
}

void Graphics::GeometryProcessor::ConvertPolygon(PolygonFormat targetPolygonFormat, const std::vector<float3>& positions, std::vector<size_t>::const_iterator positionFaceBegin,
	std::vector<size_t>::const_iterator positionFaceEnd, std::vector<size_t>& newFacesRelativeIndices)
{
	size_t targetVerticesPerFace = (targetPolygonFormat == PolygonFormat::TRIANGLE) ? 3 : 4;
	size_t vertexIndexShiftThreshould = targetVerticesPerFace - 1;
	size_t verticesCount = std::distance(positionFaceBegin, positionFaceEnd);

	SplittedMeshData newFaceData;

	RingBufferVector<size_t> freeVertexIndices(positionFaceBegin, positionFaceEnd);
	RingBufferVector<size_t> freeVertexIndexIds(freeVertexIndices.Size());
	std::iota(freeVertexIndexIds.GetNative().begin(), freeVertexIndexIds.GetNative().end(), 0);

	if (targetPolygonFormat == PolygonFormat::N_GON || verticesCount == targetVerticesPerFace)
	{
		newFacesRelativeIndices = freeVertexIndexIds.GetNative();

		return;
	}

	GeometryProcessor::RemoveRedundantVertices(positions, targetVerticesPerFace, freeVertexIndices, freeVertexIndexIds);

	float3 faceNormal = GeometryProcessor::CalculatePolygonNormal(positions, positionFaceBegin, positionFaceEnd);
	int64_t vertexIndexShift = 0;

	newFacesRelativeIndices.clear();
	newFacesRelativeIndices.reserve(freeVertexIndices.Size());

	while (freeVertexIndices.Size() > vertexIndexShiftThreshould)
	{
		int64_t freeVertexPreviousIndex;
		int64_t freeVertexCurrentIndex;
		int64_t freeVertexNextIndex;
		int64_t freeVertexNextNextIndex = 0;
		int64_t freeVertexPreviousIndexId;
		int64_t freeVertexCurrentIndexId;
		int64_t freeVertexNextIndexId;
		int64_t freeVertexNextNextIndexId = 0;

		if (static_cast<int64_t>(vertexIndexShift + vertexIndexShiftThreshould) < static_cast<int64_t>(freeVertexIndices.Size()))
		{
			freeVertexPreviousIndex = freeVertexIndices[vertexIndexShift];
			freeVertexCurrentIndex = freeVertexIndices[vertexIndexShift + 1];
			freeVertexNextIndex = freeVertexIndices[vertexIndexShift + 2];
			freeVertexPreviousIndexId = freeVertexIndexIds[vertexIndexShift];
			freeVertexCurrentIndexId = freeVertexIndexIds[vertexIndexShift + 1];
			freeVertexNextIndexId = freeVertexIndexIds[vertexIndexShift + 2];

			if (targetVerticesPerFace == 4)
			{
				freeVertexNextNextIndex = freeVertexIndices[vertexIndexShift + 3];
				freeVertexNextNextIndexId = freeVertexIndexIds[vertexIndexShift + 3];
			}
		}
		else
		{
			freeVertexPreviousIndex = freeVertexIndices[0];
			freeVertexPreviousIndexId = freeVertexIndexIds[0];

			if (targetVerticesPerFace == 3)
			{
				freeVertexCurrentIndex = freeVertexIndices[2];
				freeVertexNextIndex = freeVertexIndices[1];
				freeVertexCurrentIndexId = freeVertexIndexIds[2];
				freeVertexNextIndexId = freeVertexIndexIds[1];
			}
			else
			{
				freeVertexCurrentIndex = freeVertexIndices[3];
				freeVertexNextIndex = freeVertexIndices[2];
				freeVertexNextNextIndex = freeVertexIndices[1];
				freeVertexCurrentIndexId = freeVertexIndexIds[3];
				freeVertexNextIndexId = freeVertexIndexIds[2];
				freeVertexNextNextIndexId = freeVertexIndexIds[1];
			}
		}

		const float3& positionPrevious = positions[freeVertexPreviousIndex];
		const float3& positionCurrent = positions[freeVertexCurrentIndex];
		const float3& positionNext = positions[freeVertexNextIndex];
		const float3& positionNextNext = positions[freeVertexNextNextIndex];

		bool triangleIsIncorrect = !(GeometryProcessor::CalculateTriangleArea(positionPrevious, positionCurrent, positionNext) > 0.0f);

		if (!triangleIsIncorrect && targetVerticesPerFace == 4)
			triangleIsIncorrect = triangleIsIncorrect || !(GeometryProcessor::CalculateTriangleArea(positionPrevious, positionNext, positionNextNext) > 0.0f);

		if (!triangleIsIncorrect)
		{
			for (auto& vertexId : freeVertexIndices.GetNative())
				if (vertexId != freeVertexPreviousIndex && vertexId != freeVertexCurrentIndex && vertexId != freeVertexNextIndex)
				{
					if (GeometryProcessor::CheckPointInTriangle(positionPrevious, positionCurrent, positionNext, positions[vertexId]))
					{
						triangleIsIncorrect = true;

						break;
					}
					else
					{
						if (targetVerticesPerFace == 4)
							if (GeometryProcessor::CheckPointInTriangle(positionPrevious, positionNext, positionNextNext, positions[vertexId]))
							{
								triangleIsIncorrect = true;

								break;
							}
					}
				}
		}

		if (!triangleIsIncorrect)
		{
			if (!GeometryProcessor::CheckTriangleInPolygon(positionPrevious, positionCurrent, positionNext, faceNormal))
				triangleIsIncorrect = true;
			else
				if (targetVerticesPerFace == 4)
					if (!GeometryProcessor::CheckTriangleInPolygon(positionPrevious, positionNext, positionNextNext, faceNormal))
						triangleIsIncorrect = true;
		}

		if (triangleIsIncorrect)
			vertexIndexShift = (static_cast<int64_t>(vertexIndexShift + vertexIndexShiftThreshould) >= static_cast<int64_t>(freeVertexIndices.Size())) ? 0 : ++vertexIndexShift;
		else
		{
			newFacesRelativeIndices.push_back(freeVertexPreviousIndexId);
			newFacesRelativeIndices.push_back(freeVertexCurrentIndexId);
			newFacesRelativeIndices.push_back(freeVertexNextIndexId);

			if (targetVerticesPerFace == 4)
				newFacesRelativeIndices.push_back(freeVertexNextNextIndexId);

			int64_t vertexIndexForRemove = (static_cast<int64_t>(vertexIndexShift + vertexIndexShiftThreshould) < static_cast<int64_t>(freeVertexIndices.Size())) ? vertexIndexShift + 1 : 1;

			freeVertexIndices.RemoveElement(vertexIndexForRemove);
			freeVertexIndexIds.RemoveElement(vertexIndexForRemove);

			vertexIndexShift = 0;
		}
	}
}

void Graphics::GeometryProcessor::RemoveRedundantVertices(const std::vector<float3>& positions, size_t verticesPerFace, RingBufferVector<size_t>& vertexIndices,
	RingBufferVector<size_t>& vertexIndexIds)
{
	int64_t vertexIndexShift = -1;

	while (vertexIndices.Size() > (verticesPerFace - 1))
	{
		const float3& positionPrevious = positions[vertexIndices[vertexIndexShift]];
		const float3& positionCurrent = positions[vertexIndices[vertexIndexShift + 1]];
		const float3& positionNext = positions[vertexIndices[vertexIndexShift + 2]];

		if (!(GeometryProcessor::CalculateTriangleArea(positionPrevious, positionCurrent, positionNext) > 0.0f))
		{
			vertexIndices.RemoveElement(vertexIndexShift + 1);
			vertexIndexIds.RemoveElement(vertexIndexShift + 1);
			vertexIndexShift = 0;
		}

		if (vertexIndexShift == (vertexIndices.Size() - verticesPerFace + 1))
			break;

		vertexIndexShift++;
	}
}
