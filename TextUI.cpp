#include "TextUI.h"

Graphics::TextUI::TextUI(ID3D12Device* device, int32_t relativePositionX, int32_t relativePositionY, float2 scale, float pixelsPerHeight, float pixelsPerLetterSpacing,
	float pixelsPerLineSpacing, size_t _lineLength, float4 color, const Font* _font, UIHorizontalAlign horizontalAlign, UIVerticalAlign verticalAlign)
	: stringSize(0), lineLength(_lineLength), order(0), isActive(true), material(nullptr), font(_font)
{
	if (_font == nullptr)
		throw std::exception("Graphics::TextUI::TextUI: Font is null");

	int32_t coordOriginX = (horizontalAlign == UIHorizontalAlign::UI_ALIGN_LEFT) ? 0 : (horizontalAlign == UIHorizontalAlign::UI_ALIGN_CENTER) ?
		GraphicsSettings::GetResolutionX() / 2 : GraphicsSettings::GetResolutionX() - 1;
	int32_t coordOriginY = (verticalAlign == UIVerticalAlign::UI_ALIGN_TOP) ? 0 : (verticalAlign == UIVerticalAlign::UI_ALIGN_MIDDLE) ?
		GraphicsSettings::GetResolutionY() / 2 : GraphicsSettings::GetResolutionY() - 1;

	localConstBuffer.scale.x = scale.x;
	localConstBuffer.scale.y = scale.y;
	localConstBuffer.screenCoordOffset.x = 2.0f * (coordOriginX + relativePositionX) / static_cast<float>(GraphicsSettings::GetResolutionX()) - 1.0f;
	localConstBuffer.screenCoordOffset.y = 1.0f - 2.0f * (coordOriginY + relativePositionY) / static_cast<float>(GraphicsSettings::GetResolutionY());
	localConstBuffer.color = color;

	localConstBufferId = resourceManager.CreateConstantBuffer(&localConstBuffer, sizeof(localConstBuffer));

	vertices.resize(STRING_LENGTH_MAX, {});

	vertexBufferId = resourceManager.CreateDynamicVertexBuffer(vertices.data(), vertices.size() * sizeof(VertexData), sizeof(VertexData));
	vertexBufferView = resourceManager.GetVertexBufferView(vertexBufferId);

	letterHeight = pixelsPerHeight * 2.0f / static_cast<float>(GraphicsSettings::GetResolutionY());
	letterSpacing = pixelsPerLetterSpacing * 2.0f / static_cast<float>(GraphicsSettings::GetResolutionX());
	lineSpacing = pixelsPerLineSpacing * 2.0f / static_cast<float>(GraphicsSettings::GetResolutionY());
}

Graphics::TextUI::~TextUI()
{

}

void Graphics::TextUI::SetString(ID3D12GraphicsCommandList* commandList, std::string string)
{
	std::wstring wcharString;
	wcharString.resize(string.size());

	for (size_t charId = 0; charId < string.size(); charId++)
		wcharString[charId] = (string[charId] + 256) % 256;
	
	SetString(commandList, wcharString);
}

void Graphics::TextUI::SetString(ID3D12GraphicsCommandList* commandList, std::wstring string)
{
	stringSize = string.size();

	if (stringSize > 0)
	{
		auto& charCodeStart = font->GetCharCodeStart();
		auto& glyphBuffer = font->GetGlyphBuffer();
		auto& atlasCellSize = font->GetAtlasCellSize();
		float scaleCoeff = letterHeight / atlasCellSize.y;

		VertexData vertex{};
		size_t vertexId = 0;
		size_t charsInLine = 0;

		for (auto& textChar : string)
		{
			int64_t charCode = static_cast<int64_t>(textChar) - charCodeStart;
			
			if (textChar == ' ')
				vertex.localScreenCoordOffset.x += letterHeight + letterSpacing;

			if (charsInLine > lineLength || textChar == static_cast<wchar_t>('\n'))
			{
				vertex.localScreenCoordOffset.x = 0.0f;
				vertex.localScreenCoordOffset.y -= letterHeight + lineSpacing;
				charsInLine = 0;
			}

			if (charCode < 0 || charCode >= static_cast<int64_t>(glyphBuffer.size()))
				continue;

			charsInLine++;

			if (textChar != ' ' && textChar != static_cast<wchar_t>('\n'))
			{
				auto& glyphData = glyphBuffer[charCode];
				float localScreenCoordOffsetY = vertex.localScreenCoordOffset.y;
				
				vertex.localScreenCoordOffset.y -= glyphData.localOffset.y * glyphData.size.y * scaleCoeff * 4.0f;
				vertex.localScale = float2(glyphData.size.x * 2.0f * scaleCoeff, glyphData.size.y * 2.0f * scaleCoeff);
				vertex.texCoordOffset = glyphData.offset;
				vertex.texCoordScale = glyphData.size;

				vertices[vertexId].localScreenCoordOffset = vertex.localScreenCoordOffset;
				vertices[vertexId].localScale = vertex.localScale;
				vertices[vertexId].texCoordOffset = vertex.texCoordOffset;
				vertices[vertexId++].texCoordScale = vertex.texCoordScale;

				vertex.localScreenCoordOffset.x += glyphData.size.x * 2.0f * scaleCoeff + letterSpacing;
				vertex.localScreenCoordOffset.y = localScreenCoordOffsetY;
			}
		}

		resourceManager.UpdateDynamicVertexBuffer(vertexBufferId, vertices.data(), vertexId * sizeof(VertexData));
	}
}

void Graphics::TextUI::SetMaterial(const Material* _material) noexcept
{
	if (_material != nullptr)
		if (_material->IsComposed())
			material = _material;
}

void Graphics::TextUI::SetOrder(int64_t newOrder) noexcept
{
	order = newOrder;
}

void Graphics::TextUI::SetActive(bool active) noexcept
{
	isActive = active;
}

const int64_t& Graphics::TextUI::GetOrder() const noexcept
{
	return order;
}

const bool& Graphics::TextUI::IsActive() const noexcept
{
	return isActive;
}

Graphics::ConstantBufferId Graphics::TextUI::GetConstantBufferId() const noexcept
{
	return localConstBufferId;
}

void Graphics::TextUI::Draw(ID3D12GraphicsCommandList* commandList) const
{
	if (material != nullptr && stringSize > 0)
	{
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

		material->Present(commandList);

		commandList->DrawInstanced(4, static_cast<uint32_t>(stringSize), 0, 0);
	}
}
