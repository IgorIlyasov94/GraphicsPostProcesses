#pragma once

#include "stdafx.h"

namespace Graphics
{
	struct SplittedMeshData
	{
	public:
		std::vector<float3> positions;
		std::vector<float3> normals;
		std::vector<float3> tangents;
		std::vector<float3> binormals;
		std::vector<float2> texCoords;
		std::vector<size_t> positionFaces;
		std::vector<size_t> normalFaces;
		std::vector<size_t> texCoordFaces;
		std::vector<size_t> verticesPerFaces;

		void Clear()
		{
			positions.clear();
			normals.clear();
			texCoords.clear();
			positionFaces.clear();
			normalFaces.clear();
			texCoordFaces.clear();
			verticesPerFaces.clear();
		}
	};

	enum class VertexFormat : uint32_t
	{
		UNDEFINED = 0U,
		POSITION = 1U,
		TEXCOORD = 2U,
		NORMAL = 4U,
		TANGENT_BINORMAL = 8U
	};

	inline VertexFormat operator|(VertexFormat leftValue, VertexFormat rightValue)
	{
		using VertexFormatType = std::underlying_type_t<VertexFormat>;

		return static_cast<VertexFormat>(static_cast<VertexFormatType>(leftValue) | static_cast<VertexFormatType>(rightValue));
	}

	inline VertexFormat& operator|=(VertexFormat& leftValue, VertexFormat rightValue)
	{
		leftValue = leftValue | rightValue;

		return leftValue;
	}

	inline VertexFormat operator&(VertexFormat leftValue, VertexFormat rightValue)
	{
		using VertexFormatType = std::underlying_type_t<VertexFormat>;

		return static_cast<VertexFormat>(static_cast<VertexFormatType>(leftValue) & static_cast<VertexFormatType>(rightValue));
	}

	inline VertexFormat& operator&=(VertexFormat& leftValue, VertexFormat rightValue)
	{
		leftValue = leftValue & rightValue;

		return leftValue;
	}

	inline VertexFormat operator~(VertexFormat leftValue)
	{
		using VertexFormatType = std::underlying_type_t<VertexFormat>;

		return static_cast<VertexFormat>(~static_cast<VertexFormatType>(leftValue));
	}

	inline VertexFormat& operator~(VertexFormat& leftValue)
	{
		using VertexFormatType = std::underlying_type_t<VertexFormat>;

		leftValue = static_cast<VertexFormat>(~static_cast<VertexFormatType>(leftValue));

		return leftValue;
	}

	inline size_t VertexStride(VertexFormat format) noexcept
	{
		size_t vertexStride = 0;

		if ((format & VertexFormat::POSITION) == VertexFormat::POSITION)
			vertexStride += 12;

		if ((format & VertexFormat::NORMAL) == VertexFormat::NORMAL)
			vertexStride += 12;

		if ((format & VertexFormat::TANGENT_BINORMAL) == VertexFormat::TANGENT_BINORMAL)
			vertexStride += 24;

		if ((format & VertexFormat::TEXCOORD) == VertexFormat::TEXCOORD)
			vertexStride += 8;

		return vertexStride;
	}

	struct Vertex
	{
	public:
		float3 position;
		float3 normal;
		float3 tangent;
		float3 binormal;
		float2 texCoord;
		VertexFormat format;

		size_t GetStride() noexcept
		{
			return VertexStride(format);
		}

		void CopyToRawChunk(std::vector<uint32_t>& chunk)
		{
			chunk.clear();
			chunk.reserve(GetStride() / 4);

			if ((format & VertexFormat::POSITION) == VertexFormat::POSITION)
				std::copy(reinterpret_cast<uint32_t*>(&position), reinterpret_cast<uint32_t*>(&position) + 3, std::back_inserter(chunk));

			if ((format & VertexFormat::NORMAL) == VertexFormat::NORMAL)
				std::copy(reinterpret_cast<uint32_t*>(&normal), reinterpret_cast<uint32_t*>(&normal) + 3, std::back_inserter(chunk));

			if ((format & VertexFormat::TANGENT_BINORMAL) == VertexFormat::TANGENT_BINORMAL)
			{
				std::copy(reinterpret_cast<uint32_t*>(&tangent), reinterpret_cast<uint32_t*>(&tangent) + 3, std::back_inserter(chunk));
				std::copy(reinterpret_cast<uint32_t*>(&binormal), reinterpret_cast<uint32_t*>(&binormal) + 3, std::back_inserter(chunk));
			}

			if ((format & VertexFormat::TEXCOORD) == VertexFormat::TEXCOORD)
				std::copy(reinterpret_cast<uint32_t*>(&texCoord), reinterpret_cast<uint32_t*>(&texCoord) + 2, std::back_inserter(chunk));
		}
	};

	inline bool operator==(const Vertex& leftValue, const Vertex& rightValue)
	{
		if (leftValue.format != rightValue.format)
			return false;

		if ((leftValue.format & VertexFormat::POSITION) == VertexFormat::POSITION)
			if (!XMVector3Equal(XMLoadFloat3(&leftValue.position), XMLoadFloat3(&rightValue.position)))
				return false;

		if ((leftValue.format & VertexFormat::NORMAL) == VertexFormat::NORMAL)
			if (!XMVector3Equal(XMLoadFloat3(&leftValue.normal), XMLoadFloat3(&rightValue.normal)))
				return false;

		if ((leftValue.format & VertexFormat::TANGENT_BINORMAL) == VertexFormat::TANGENT_BINORMAL)
		{
			if (!XMVector3Equal(XMLoadFloat3(&leftValue.tangent), XMLoadFloat3(&rightValue.tangent)))
				return false;

			if (!XMVector3Equal(XMLoadFloat3(&leftValue.binormal), XMLoadFloat3(&rightValue.binormal)))
				return false;
		}

		if ((leftValue.format & VertexFormat::TEXCOORD) == VertexFormat::TEXCOORD)
			if (!XMVector2Equal(XMLoadFloat2(&leftValue.texCoord), XMLoadFloat2(&rightValue.texCoord)))
				return false;

		return true;
	}

	enum class PolygonFormat
	{
		TRIANGLE,
		QUAD,
		N_GON
	};
}
