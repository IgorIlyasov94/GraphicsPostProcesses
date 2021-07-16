#pragma once

#include "GraphicsSettings.h"
#include "ResourceManager.h"
#include "Font.h"
#include "Material.h"

namespace Graphics
{
	class TextUI
	{
	public:
		TextUI(ID3D12Device* device, int32_t relativePositionX, int32_t relativePositionY, float2 scale, float pixelsPerHeight, float pixelsPerLetterSpacing,
			float pixelsPerLineSpacing, size_t _lineLength, float4 color, const Font* _font, UIHorizontalAlign horizontalAlign = UIHorizontalAlign::UI_ALIGN_LEFT,
			UIVerticalAlign verticalAlign = UIVerticalAlign::UI_ALIGN_TOP);
		~TextUI();

		void SetString(ID3D12GraphicsCommandList* commandList, std::string string);
		void SetString(ID3D12GraphicsCommandList* commandList, std::wstring string);
		void SetMaterial(const Material* _material) noexcept;
		void SetOrder(int64_t newOrder) noexcept;
		void SetActive(bool active) noexcept;

		const int64_t& GetOrder() const noexcept;
		const bool& IsActive() const noexcept;

		ConstantBufferId GetConstantBufferId() const noexcept;

		void Draw(ID3D12GraphicsCommandList* commandList) const;

	private:
		TextUI() = delete;

		VertexBufferId vertexBufferId;
		ConstantBufferId localConstBufferId;

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

		struct LocalConstBuffer
		{
			float2 screenCoordOffset;
			float2 scale;
			float4 color;
		};

		struct VertexData
		{
			float2 localScreenCoordOffset;
			float2 localScale;
			float2 texCoordOffset;
			float2 texCoordScale;
		};

		LocalConstBuffer localConstBuffer;

		float letterHeight;
		float letterSpacing;
		float lineSpacing;
		size_t stringSize;
		size_t lineLength;

		static const size_t STRING_LENGTH_MAX = 2048;

		int64_t order;
		bool isActive;

		std::vector<VertexData> vertices;

		const Material* material;
		const Font* font;

		ResourceManager& resourceManager = ResourceManager::GetInstance();
	};
}
