#pragma once

#include "ResourceManager.h"
#include "OBJLoader.h"

namespace Graphics
{
	class Mesh
	{
	public:
		Mesh(std::filesystem::path filePath);
		Mesh(VertexFormat _vertexFormat, const void* verticesData, size_t verticesDataSize, const void* indicesData, size_t indicesDataSize);
		~Mesh();

		uint32_t GetIndicesCount() const noexcept;
		VertexFormat GetVertexFormat() const noexcept;

		void Present(ID3D12GraphicsCommandList* commandList) const;

	private:
		Mesh() = delete;

		VertexBufferId vertexBufferId;
		IndexBufferId indexBufferId;

		uint32_t indicesCount;

		VertexFormat vertexFormat;

		const D3D12_VERTEX_BUFFER_VIEW* vertexBufferView;
		const D3D12_INDEX_BUFFER_VIEW* indexBufferView;

		ResourceManager& resourceManager = ResourceManager::GetInstance();
	};
}
