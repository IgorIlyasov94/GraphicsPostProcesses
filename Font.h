#pragma once

#include "ResourceManager.h"

namespace Graphics
{
	using GlyphData = struct
	{
		float2 offset;
		float2 localOffset;
		float2 size;
	};

	class Font
	{
	public:
		Font(TextureId _fontAtlasId, size_t atlasColumnNumber, size_t atlasRowNumber, wchar_t firstCharInAtlas);
		~Font();

		const TextureId& GetFontAtlasId() const noexcept;
		const size_t& GetCharCodeStart() const noexcept;
		const std::vector<GlyphData>& GetGlyphBuffer() const noexcept;
		const float2& GetAtlasCellSize() const noexcept;

	private:
		Font() = delete;

		void GetGlyphDataFromTextureRegion(size_t columnId, size_t rowId, size_t atlasColumnNumber, size_t atlasRowNumber, size_t textureWidth, size_t textureHeight,
			const std::vector<float4>& textureData, GlyphData& result);

		TextureId fontAtlasId;
		size_t charCodeStart;
		size_t columnNumber;
		size_t rowNumber;

		float2 atlasCellSize;

		std::vector<GlyphData> glyphBuffer;

		ResourceManager& resourceManager = ResourceManager::GetInstance();
	};
}
