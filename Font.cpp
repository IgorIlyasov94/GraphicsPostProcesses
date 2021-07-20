#include "Font.h"

Graphics::Font::Font(TextureId _fontAtlasId, size_t atlasColumnNumber, size_t atlasRowNumber, wchar_t firstCharInAtlas)
	: fontAtlasId(_fontAtlasId), charCodeStart(firstCharInAtlas), columnNumber(atlasColumnNumber), rowNumber(atlasRowNumber)
{
	auto& textureInfo = resourceManager.GetTexture(fontAtlasId).info;

	std::vector<float4> textureData;
	resourceManager.GetTextureDataFromGPU(fontAtlasId, textureData);

	glyphBuffer.resize(atlasColumnNumber * atlasRowNumber, {});

	atlasCellSize = { 1.0f / atlasColumnNumber, 1.0f / atlasRowNumber };

	for (size_t rowId = 0; rowId < atlasRowNumber; rowId++)
		for (size_t columnId = 0; columnId < atlasColumnNumber; columnId++)
		{
			GlyphData glyphData{};
			GetGlyphDataFromTextureRegion(columnId, rowId, atlasColumnNumber, atlasRowNumber, textureInfo.width, textureInfo.height, textureData, glyphData);

			glyphBuffer[rowId * atlasRowNumber + columnId] = glyphData;
		}
}

Graphics::Font::~Font()
{

}

const Graphics::TextureId& Graphics::Font::GetFontAtlasId() const noexcept
{
	return fontAtlasId;
}

const size_t& Graphics::Font::GetCharCodeStart() const noexcept
{
	return charCodeStart;
}

const std::vector<Graphics::GlyphData>& Graphics::Font::GetGlyphBuffer() const noexcept
{
	return glyphBuffer;
}

const float2& Graphics::Font::GetAtlasCellSize() const noexcept
{
	return atlasCellSize;
}

void Graphics::Font::GetGlyphDataFromTextureRegion(size_t columnId, size_t rowId, size_t atlasColumnNumber, size_t atlasRowNumber, size_t textureWidth, size_t textureHeight,
	const std::vector<float4>& textureData, GlyphData& result)
{
	size_t atlasCellWidth = textureWidth / atlasColumnNumber;
	size_t atlasCellHeight = textureHeight / atlasRowNumber;

	int64_t startTextureColumn = columnId * atlasCellWidth;
	int64_t startTextureRow = rowId * atlasCellHeight;
	int64_t endTextureColumn = startTextureColumn + atlasCellWidth - 1;
	int64_t endTextureRow = startTextureRow + atlasCellHeight - 1;
	int64_t minColumn = endTextureColumn;
	int64_t maxColumn = startTextureColumn;
	int64_t minRow = endTextureRow;
	int64_t maxRow = startTextureRow;

	for (int64_t rowId = startTextureRow; rowId <= endTextureRow; rowId++)
		for (int64_t columnId = startTextureColumn; columnId <= endTextureColumn; columnId++)
		{
			size_t pixelIndex = rowId * textureWidth + columnId;

			if (textureData[pixelIndex].w > 0.0f)
			{
				if (minColumn > columnId)
					minColumn = columnId;

				if (minRow > rowId)
					minRow = rowId;
			}
		}

	for (int64_t rowId = endTextureRow; rowId >= startTextureRow; rowId--)
		for (int64_t columnId = endTextureColumn; columnId >= startTextureColumn; columnId--)
		{
			size_t pixelIndex = rowId * textureWidth + columnId;

			if (textureData[pixelIndex].w > 0.0f)
			{
				if (maxColumn < columnId)
					maxColumn = columnId;

				if (maxRow < rowId)
					maxRow = rowId;
			}
		}

	result.offset = { (minColumn - 1) / static_cast<float>(textureWidth), (minRow - 1) / static_cast<float>(textureHeight) };
	result.localOffset.x = std::max(minColumn - startTextureColumn - 1, 0i64) / static_cast<float>(atlasCellWidth);
	result.localOffset.y = std::max(minRow - startTextureRow - 1, 0i64) / static_cast<float>(atlasCellHeight);
	result.size.x = (maxColumn - minColumn + 2) / static_cast<float>(textureWidth);
	result.size.y = (maxRow - minRow + 2) / static_cast<float>(textureHeight);
}
