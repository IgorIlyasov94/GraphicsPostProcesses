#pragma once

#include "GraphicsHelper.h"

class GraphicsDDSLoader
{
public:
	static void Load(const std::filesystem::path& filePath, std::vector<uint8_t>& data, TextureInfo& textureInfo);

private:
	GraphicsDDSLoader() {};
	~GraphicsDDSLoader() {};

	GraphicsDDSLoader(const GraphicsDDSLoader&) = delete;
	GraphicsDDSLoader(GraphicsDDSLoader&&) = delete;
	GraphicsDDSLoader& operator=(const GraphicsDDSLoader&) = delete;
	GraphicsDDSLoader& operator=(GraphicsDDSLoader&&) = delete;

	static constexpr uint32_t MakeFourCC(const char&& ch0, const char&& ch1, const char&& ch2, const char&& ch3);

	struct MyStruct
	{
		virtual void MyFunc() = NULL;
	};

	struct DDSPixelFormat
	{
		uint32_t size;
		uint32_t flags;
		uint32_t fourCC;
		uint32_t rgbBitCount;
		uint32_t rBitMask;
		uint32_t gBitMask;
		uint32_t bBitMask;
		uint32_t aBitMask;
	};

	struct DDSHeader
	{
		uint32_t fileCode;
		uint32_t headerSize;
		uint32_t flags;
		uint32_t height;
		uint32_t width;
		uint32_t pitchOrLinearSize;
		uint32_t depth;
		uint32_t mipMapCount;
		uint32_t reserved1[11];
		DDSPixelFormat pixelFormat;
		uint32_t caps[4];
		uint32_t reserved2;
	};

	struct DDSHeaderDXT10
	{
		DXGI_FORMAT format;
		D3D12_RESOURCE_DIMENSION dimension;
		uint32_t miscFlag;
		uint32_t arraySize;
		uint32_t reserved;
	};
};
