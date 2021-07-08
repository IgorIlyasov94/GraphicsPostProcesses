#pragma once

#include "GraphicsSettings.h"
#include "ResourceManager.h"
#include "Material.h"

namespace Graphics
{
	class SpriteUI
	{
	public:
		SpriteUI(int32_t relativePositionX, int32_t relativePositionY, float2 scale, float4 color, TextureId spriteMainTextureId, const Material* _material,
			bool preciseSpriteMesh = false, float2 spriteOrigin = { 0.5f, 0.5f }, UIHorizontalAlign horizontalAlign = UIHorizontalAlign::UI_ALIGN_LEFT,
			UIVerticalAlign verticalAlign = UIVerticalAlign::UI_ALIGN_TOP);
		~SpriteUI();

		void SetOrder(int64_t newOrder) noexcept;
		void SetActive(bool active) noexcept;
		
		const int64_t& GetOrder() const noexcept;
		const bool& IsActive() const noexcept;

		bool PointInsideMesh(float2 point) const;

		ConstantBufferId GetConstantBufferId() const noexcept;

		void Draw(ID3D12GraphicsCommandList* commandList) const;

	private:
		SpriteUI() = delete;

		void GenerateMeshFromTexture(TextureId textureId, size_t gridDensityX, size_t gridDensityY, VertexBufferId& resultVertexBufferId,
			IndexBufferId& resultIndexBufferId, std::vector<float3>& resultVertices, std::vector<uint32_t>& resultIndices);

		void FindFirstAlphaNonZeroPoints(int32_t directionX, int32_t directionY, size_t gridDensityX, size_t gridDensityY, size_t textureWidth, size_t textureHeight,
			bool isVertical, const std::vector<float4>& textureData, std::vector<float3>& resultVertices);

		VertexBufferId vertexBufferId;
		IndexBufferId indexBufferId;
		ConstantBufferId localConstBufferId;

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		D3D12_INDEX_BUFFER_VIEW indexBufferView;

		struct LocalConstBuffer
		{
			float2 screenCoordOffset;
			float2 scale;
			float4 color;
		};

		LocalConstBuffer localConstBuffer;

		std::vector<float3> vertices;
		std::vector<uint32_t> indices;

		TextureId spriteTextureId;
		int64_t order;
		bool isActive;

		const Material* material;

		ResourceManager& resourceManager = ResourceManager::GetInstance();
	};
}
