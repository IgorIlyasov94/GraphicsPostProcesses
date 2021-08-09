#include "SpriteUI.h"
#include "GeometryProcessor.h"

Graphics::SpriteUI::SpriteUI(int32_t relativePositionX, int32_t relativePositionY, float2 scale, float4 color, TextureId spriteMainTextureId,
	bool preciseSpriteMesh, float2 spriteOrigin, UIHorizontalAlign horizontalAlign, UIVerticalAlign verticalAlign)
	: localConstBuffer{}, order(0), isActive(true), material(nullptr)
{
	int32_t coordOriginX = (horizontalAlign == UIHorizontalAlign::UI_ALIGN_LEFT) ? 0 : (horizontalAlign == UIHorizontalAlign::UI_ALIGN_CENTER) ?
		GraphicsSettings::GetResolutionX() / 2 : GraphicsSettings::GetResolutionX() - 1;
	int32_t coordOriginY = (verticalAlign == UIVerticalAlign::UI_ALIGN_TOP) ? 0 : (verticalAlign == UIVerticalAlign::UI_ALIGN_MIDDLE) ?
		GraphicsSettings::GetResolutionY() / 2 : GraphicsSettings::GetResolutionY() - 1;

	auto& textureInfo = resourceManager.GetTexture(spriteMainTextureId).info;

	localConstBuffer.scale.x = 2.0f * textureInfo.width * scale.x / static_cast<float>(GraphicsSettings::GetResolutionX());
	localConstBuffer.scale.y = -2.0f * textureInfo.height * scale.y / static_cast<float>(GraphicsSettings::GetResolutionY());
	localConstBuffer.screenCoordOffset.x = 2.0f * (coordOriginX + relativePositionX) / static_cast<float>(GraphicsSettings::GetResolutionX()) - 1.0f;
	localConstBuffer.screenCoordOffset.x -= spriteOrigin.x * localConstBuffer.scale.x;
	localConstBuffer.screenCoordOffset.y = 1.0f - 2.0f * (coordOriginY + relativePositionY) / static_cast<float>(GraphicsSettings::GetResolutionY());
	localConstBuffer.screenCoordOffset.y -= spriteOrigin.y * localConstBuffer.scale.y;
	localConstBuffer.color = color;

	localConstBufferId = resourceManager.CreateConstantBuffer(&localConstBuffer, sizeof(localConstBuffer));

	spriteTextureId = spriteMainTextureId;

	size_t gridDensityX = (preciseSpriteMesh) ? textureInfo.width : 4;
	size_t gridDensityY = (preciseSpriteMesh) ? textureInfo.height : 4;

	GenerateMeshFromTexture(spriteMainTextureId, gridDensityX, gridDensityY, vertexBufferId, indexBufferId, vertices, indices);

	vertexBufferView = resourceManager.GetVertexBufferView(vertexBufferId);
	indexBufferView = resourceManager.GetIndexBufferView(indexBufferId);
}

Graphics::SpriteUI::~SpriteUI()
{

}

void Graphics::SpriteUI::SetMaterial(const Material* _material) noexcept
{
	if (_material != nullptr)
		if (_material->IsComposed())
			material = _material;
}

void Graphics::SpriteUI::SetOrder(int64_t newOrder) noexcept
{
	order = newOrder;
}

void Graphics::SpriteUI::SetActive(bool active) noexcept
{
	isActive = active;
}

const int64_t& Graphics::SpriteUI::GetOrder() const noexcept
{
	return order;
}

const bool& Graphics::SpriteUI::IsActive() const noexcept
{
	return isActive;
}

bool Graphics::SpriteUI::PointInsideMesh(float2 point) const
{
	for (size_t triangleId = 0; triangleId < indices.size() / 3; triangleId++)
	{
		float3 point0 = vertices[indices[triangleId * 3]];
		point0.x = point0.x * localConstBuffer.scale.x + localConstBuffer.screenCoordOffset.x;
		point0.y = point0.y * localConstBuffer.scale.y + localConstBuffer.screenCoordOffset.y;

		float3 point1 = vertices[indices[triangleId * 3 + 1]];
		point1.x += point1.x * localConstBuffer.scale.x + localConstBuffer.screenCoordOffset.x;
		point1.y += point1.y * localConstBuffer.scale.y + localConstBuffer.screenCoordOffset.y;

		float3 point2 = vertices[indices[triangleId * 3 + 2]];
		point2.x = point2.x * localConstBuffer.scale.x + localConstBuffer.screenCoordOffset.x;
		point2.y = point2.y * localConstBuffer.scale.y + localConstBuffer.screenCoordOffset.y;

		if (GeometryProcessor::CheckPointInTriangle(point0, point1, point2, { point.x, point.y, 0.0f }))
			return true;
	}

	return false;
}

Graphics::ConstantBufferId Graphics::SpriteUI::GetConstantBufferId() const noexcept
{
	return localConstBufferId;
}

void Graphics::SpriteUI::Draw(ID3D12GraphicsCommandList* commandList) const
{
	if (material != nullptr)
	{
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
		commandList->IASetIndexBuffer(&indexBufferView);

		material->Present(commandList);

		commandList->DrawIndexedInstanced(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
	}
}

void Graphics::SpriteUI::GenerateMeshFromTexture(TextureId textureId, size_t gridDensityX, size_t gridDensityY, VertexBufferId& resultVertexBufferId,
	IndexBufferId& resultIndexBufferId, std::vector<float3>& resultVertices, std::vector<uint32_t>& resultIndices)
{
	auto textureInfo = resourceManager.GetTexture(textureId).info;

	std::vector<float4> textureData;
	resourceManager.GetTextureDataFromGPU(textureId, textureData);

	resultVertices.clear();
	
	FindSpriteSilhouetteVertices(textureInfo.width, textureInfo.height, textureData, resultVertices);

	std::vector<size_t> tempIndices(resultVertices.size());
	std::iota(tempIndices.begin(), tempIndices.end(), 0);
	std::vector<size_t> relativeIndices;

	GeometryProcessor::ConvertPolygon(PolygonFormat::TRIANGLE, resultVertices, tempIndices.begin(), tempIndices.end(), relativeIndices);

	resultIndices.clear();
	resultIndices.reserve(resultVertices.size());

	for (auto& relativeIndex : relativeIndices)
		resultIndices.push_back(static_cast<uint32_t>(relativeIndex));

	resultVertexBufferId = resourceManager.CreateVertexBuffer(resultVertices.data(), resultVertices.size() * sizeof(float3), sizeof(float3));
	resultIndexBufferId = resourceManager.CreateIndexBuffer(resultIndices.data(), resultIndices.size() * sizeof(uint32_t), sizeof(uint32_t));
}

void Graphics::SpriteUI::FindSpriteSilhouetteVertices(size_t textureWidth, size_t textureHeight, const std::vector<float4>& textureData, std::vector<float3>& resultVertices)
{
	float3 point{};

	for (size_t startPixelId = 0; startPixelId < textureWidth; startPixelId++)
	{
		if (FindFirstAlphaNonZeroPoint(startPixelId, 0, textureWidth, textureHeight, textureData, point))
		{
			auto predicateVectorEquality = [point](float3 vertex) { return XMVector3Equal(XMLoadFloat3(&vertex), XMLoadFloat3(&point)); };

			if (std::find_if(resultVertices.begin(), resultVertices.end(), predicateVectorEquality) == resultVertices.end())
				resultVertices.push_back(point);
		}
	}

	for (size_t startPixelId = 1; startPixelId < textureHeight; startPixelId++)
	{
		if (FindFirstAlphaNonZeroPoint(textureWidth - 1, startPixelId, textureWidth, textureHeight, textureData, point))
		{
			auto predicateVectorEquality = [point](float3 vertex) { return XMVector3Equal(XMLoadFloat3(&vertex), XMLoadFloat3(&point)); };

			if (std::find_if(resultVertices.begin(), resultVertices.end(), predicateVectorEquality) == resultVertices.end())
				resultVertices.push_back(point);
		}
	}

	for (int64_t startPixelId = textureWidth - 2; startPixelId >= 0; startPixelId--)
	{
		if (FindFirstAlphaNonZeroPoint(startPixelId, textureHeight - 1, textureWidth, textureHeight, textureData, point))
		{
			auto predicateVectorEquality = [point](float3 vertex) { return XMVector3Equal(XMLoadFloat3(&vertex), XMLoadFloat3(&point)); };

			if (std::find_if(resultVertices.begin(), resultVertices.end(), predicateVectorEquality) == resultVertices.end())
				resultVertices.push_back(point);
		}
	}

	for (int64_t startPixelId = textureHeight - 2; startPixelId > 0; startPixelId--)
	{
		if (FindFirstAlphaNonZeroPoint(0, startPixelId, textureWidth, textureHeight, textureData, point))
		{
			auto predicateVectorEquality = [point](float3 vertex) { return XMVector3Equal(XMLoadFloat3(&vertex), XMLoadFloat3(&point)); };

			if (std::find_if(resultVertices.begin(), resultVertices.end(), predicateVectorEquality) == resultVertices.end())
				resultVertices.push_back(point);
		}
	}
}

bool Graphics::SpriteUI::FindFirstAlphaNonZeroPoint(size_t startPixelColumn, size_t startPixelRow, size_t textureWidth, size_t textureHeight,
	const std::vector<float4>& textureData, float3& resultPoint)
{
	int64_t endPixelColumn = textureWidth - startPixelColumn - 1;
	int64_t endPixelRow = textureHeight - startPixelRow - 1;
	int64_t columnDistance = endPixelColumn - startPixelColumn;
	int64_t rowDistance = endPixelRow - startPixelRow;
	float length = std::sqrt(static_cast<float>(columnDistance * columnDistance + rowDistance * rowDistance));
	float pixelMomentIncrementor = 1.0f / length;
	float2 currentPixel = { std::lerp(static_cast<float>(startPixelColumn), static_cast<float>(endPixelColumn), 0.0f), 
		std::lerp(static_cast<float>(startPixelRow), static_cast<float>(endPixelRow), 0.0f) };
	float2 previousPixel = { std::lerp(static_cast<float>(startPixelColumn), static_cast<float>(endPixelColumn), -pixelMomentIncrementor),
		std::lerp(static_cast<float>(startPixelRow), static_cast<float>(endPixelRow), -pixelMomentIncrementor) };

	for (float currentPixelMoment = 0.0f; currentPixelMoment <= 1.0f; currentPixelMoment += pixelMomentIncrementor)
	{
		currentPixel = { std::lerp(static_cast<float>(startPixelColumn), static_cast<float>(endPixelColumn), currentPixelMoment),
			std::lerp(static_cast<float>(startPixelRow), static_cast<float>(endPixelRow), currentPixelMoment) };
		size_t pixelIndex = static_cast<int64_t>(currentPixel.y) * textureWidth + static_cast<int64_t>(currentPixel.x);

		if (textureData[pixelIndex].w > 0.0f)
		{
			resultPoint.x = static_cast<int64_t>(previousPixel.x) / static_cast<float>(textureWidth - 1);
			resultPoint.y = static_cast<int64_t>(previousPixel.y) / static_cast<float>(textureHeight - 1);
			resultPoint.z = 0.0f;

			return true;
		}

		previousPixel = currentPixel;
	}

	return false;
}
