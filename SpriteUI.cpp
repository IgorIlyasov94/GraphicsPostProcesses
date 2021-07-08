#include "SpriteUI.h"

Graphics::SpriteUI::SpriteUI(int32_t relativePositionX, int32_t relativePositionY, float2 scale, float4 color, TextureId spriteMainTextureId, const Material* _material,
	bool preciseSpriteMesh, float2 spriteOrigin, UIHorizontalAlign horizontalAlign, UIVerticalAlign verticalAlign)
	: localConstBuffer{}, order(0), isActive(true), material(nullptr)
{
	int32_t coordOriginX = (horizontalAlign == UIHorizontalAlign::UI_ALIGN_LEFT) ? 0 : (horizontalAlign == UIHorizontalAlign::UI_ALIGN_CENTER) ?
		GraphicsSettings::GetResolutionX() / 2 : GraphicsSettings::GetResolutionX() - 1;
	int32_t coordOriginY = (verticalAlign == UIVerticalAlign::UI_ALIGN_TOP) ? 0 : (verticalAlign == UIVerticalAlign::UI_ALIGN_MIDDLE) ?
		GraphicsSettings::GetResolutionY() / 2 : GraphicsSettings::GetResolutionY() - 1;

	auto textureInfo = resourceManager.GetTexture(spriteMainTextureId).info;

	localConstBuffer.scale.x = 2.0f * textureInfo.width / static_cast<float>(GraphicsSettings::GetResolutionX());
	localConstBuffer.scale.y = 2.0f * textureInfo.height / static_cast<float>(GraphicsSettings::GetResolutionY());
	localConstBuffer.screenCoordOffset.x = 2.0f * (coordOriginX + relativePositionX) / static_cast<float>(GraphicsSettings::GetResolutionX()) - 1.0f;
	localConstBuffer.screenCoordOffset.x -= spriteOrigin.x * localConstBuffer.scale.x;
	localConstBuffer.screenCoordOffset.y = 1.0f - 2.0f * (coordOriginY + relativePositionY) / static_cast<float>(GraphicsSettings::GetResolutionY());
	localConstBuffer.screenCoordOffset.y += spriteOrigin.y * localConstBuffer.scale.y;
	localConstBuffer.color = color;

	localConstBufferId = resourceManager.CreateConstantBuffer(&localConstBuffer, sizeof(localConstBuffer));

	spriteTextureId = spriteMainTextureId;

	if (_material != nullptr)
		if (_material->IsComposed())
			material = _material;

	size_t gridDensityX = (preciseSpriteMesh) ? textureInfo.width : 4;
	size_t gridDensityY = (preciseSpriteMesh) ? textureInfo.height : 4;

	GenerateMeshFromTexture(spriteMainTextureId, gridDensityX, gridDensityY, vertexBufferId, indexBufferId, vertices, indices);

	vertexBufferView = resourceManager.GetVertexBufferView(vertexBufferId);
	indexBufferView = resourceManager.GetIndexBufferView(indexBufferId);
}

Graphics::SpriteUI::~SpriteUI()
{

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

		if (CheckPointInTriangle(point0, point1, point2, { point.x, point.y, 0.0f }))
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

		commandList->DrawIndexedInstanced(indices.size(), 1, 0, 0, 0);
	}
}

void Graphics::SpriteUI::GenerateMeshFromTexture(TextureId textureId, size_t gridDensityX, size_t gridDensityY, VertexBufferId& resultVertexBufferId,
	IndexBufferId& resultIndexBufferId, std::vector<float3>& resultVertices, std::vector<uint32_t>& resultIndices)
{
	auto textureInfo = resourceManager.GetTexture(textureId).info;

	std::vector<float4> textureData;
	resourceManager.GetTextureDataFromGPU(textureId, textureData);

	resultVertices.clear();
	
	FindFirstAlphaNonZeroPoints(1, 1, gridDensityX, gridDensityY, textureInfo.width, textureInfo.height, true, textureData, resultVertices);
	FindFirstAlphaNonZeroPoints(1, -1, gridDensityX, gridDensityY, textureInfo.width, textureInfo.height, true, textureData, resultVertices);
	FindFirstAlphaNonZeroPoints(1, 1, gridDensityX, gridDensityY, textureInfo.width, textureInfo.height, false, textureData, resultVertices);
	FindFirstAlphaNonZeroPoints(-1, 1, gridDensityX, gridDensityY, textureInfo.width, textureInfo.height, false, textureData, resultVertices);
	
	resultIndices.clear();
	resultIndices.resize(resultVertices.size());
	std::iota(resultIndices.begin(), resultIndices.end(), 0);

	TriangulateFace(VertexFormat::POSITION, resultVertices, resultIndices);

	resultVertexBufferId = resourceManager.CreateVertexBuffer(resultVertices.data(), resultVertices.size() * sizeof(float3), sizeof(float3));
	resultIndexBufferId = resourceManager.CreateIndexBuffer(resultIndices.data(), resultIndices.size() * sizeof(uint32_t), sizeof(uint32_t));
}

void Graphics::SpriteUI::FindFirstAlphaNonZeroPoints(int32_t directionX, int32_t directionY, size_t gridDensityX, size_t gridDensityY, size_t textureWidth,
	size_t textureHeight, bool isVertical, const std::vector<float4>& textureData, std::vector<float3>& resultVertices)
{
	if (!isVertical)
		std::swap(directionX, directionY);

	auto normalizedDirectionX = (directionX < 0) ? -1 : 1;
	auto normalizedDirectionY = (directionY < 0) ? -1 : 1;
	auto columnStart = (directionX < 0) ? gridDensityX - 1 : 0;
	auto rowStart = (directionY < 0) ? gridDensityY - 1 : 0;

	for (int64_t columnId = columnStart; (directionX < 0) ? columnId >= 0 : columnId < gridDensityX; columnId += normalizedDirectionX)
	{
		for (int64_t rowId = rowStart; (directionY < 0) ? rowId >= 0 : rowId < gridDensityY; rowId += normalizedDirectionY)
		{
			float2 coord = { columnId / static_cast<float>(gridDensityX), rowId / static_cast<float>(gridDensityY) };

			if (!isVertical)
				std::swap(coord.x, coord.y);

			auto index = static_cast<size_t>(std::floor(coord.x * textureHeight));
			index *= textureWidth;
			index += static_cast<size_t>(std::floor(coord.y * textureWidth));

			if (textureData[index].w > 0.0f)
			{
				resultVertices.push_back({ coord.x, coord.y, 0.0f });

				break;
			}
		}
	}
}
