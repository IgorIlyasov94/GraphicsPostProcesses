#include "GraphicsDDSLoader.h"

void GraphicsDDSLoader::Load(const std::filesystem::path& filePath, std::vector<uint8_t>& data, TextureInfo& textureInfo)
{
	std::ifstream ddsFile(filePath, std::ios::binary | std::ios::in);

	DDSHeader header{};

	ddsFile.read(reinterpret_cast<char*>(&header), sizeof(DDSHeader));

	bool hasHeaderDXT10 = false;
	DDSHeaderDXT10 headerDXT10{};

	textureInfo.width = header.width;
	textureInfo.height = header.height;
	textureInfo.depth = header.depth;

	if (MakeFourCC('D', 'X', '1', '0') == header.pixelFormat.fourCC)
	{
		hasHeaderDXT10 = true;
		ddsFile.read(reinterpret_cast<char*>(&headerDXT10), sizeof(DDSHeaderDXT10));

		textureInfo.format = headerDXT10.format;
		textureInfo.dimension = headerDXT10.dimension;

		auto arraySize = headerDXT10.arraySize;

		if (arraySize > 1)
			textureInfo.depth = arraySize;

		if (textureInfo.dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D)
		{
			if (arraySize > 1)
				textureInfo.srvDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
			else
				textureInfo.srvDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
		}
		else if (textureInfo.dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
		{
			if ((header.flags & D3D11_RESOURCE_MISC_TEXTURECUBE) > 0)
				if (arraySize > 1)
					textureInfo.srvDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
				else
					textureInfo.srvDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			else
				if (arraySize > 1)
					textureInfo.srvDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				else
					textureInfo.srvDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		}
		else if (textureInfo.dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
		{
			textureInfo.srvDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
		}
	}
	else
	{
		textureInfo.format = DXGI_FORMAT_UNKNOWN;
		textureInfo.dimension = D3D12_RESOURCE_DIMENSION_UNKNOWN;
		textureInfo.srvDimension = D3D12_SRV_DIMENSION_UNKNOWN;
	}

	std::vector<uint8_t> textureData{ std::istream_iterator<uint8_t>(ddsFile),std::istream_iterator<uint8_t>() };

	data = textureData;
}

constexpr uint32_t GraphicsDDSLoader::MakeFourCC(const char&& ch0, const char&& ch1, const char&& ch2, const char&& ch3)
{
	return static_cast<uint32_t>(ch0) | static_cast<uint32_t>(ch1) << 8 | static_cast<uint32_t>(ch2) << 16 | static_cast<uint32_t>(ch3) << 24;
}
