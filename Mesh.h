#pragma once

#include "ResourceManager.h"
#include "OBJLoader.h"

namespace Graphics
{
	class Mesh
	{
	public:
		Mesh(std::filesystem::path filePath);
		Mesh(VertexFormat vertexFormat, std::vector<uint8_t> verticesData, std::vector<uint8_t> indicesData);
		~Mesh();

		uint32_t GetIndicesCount();

		void Present(ID3D12GraphicsCommandList* commandList);

	private:
		Mesh() = delete;

		VertexBufferId vertexBufferId;
		IndexBufferId indexBufferId;

		uint32_t indicesCount;

		const D3D12_VERTEX_BUFFER_VIEW* vertexBufferView;
		const D3D12_INDEX_BUFFER_VIEW* indexBufferView;

		ResourceManager& resourceManager = ResourceManager::GetInstance();
	};
}
