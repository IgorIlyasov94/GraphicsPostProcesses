#pragma once

#include "ResourceManager.h"
#include "Material.h"
#include "IRenderable.h"
#include "OBJLoader.h"

namespace Graphics
{
	class Mesh final : public IRenderable
	{
	public:
		Mesh(std::filesystem::path filePath, bool calculateNormals,bool calculateTangents, bool smoothNormals);
		Mesh(VertexFormat _vertexFormat, const void* verticesData, size_t verticesDataSize, const void* indicesData, size_t indicesDataSize);
		~Mesh();

		uint32_t GetIndicesCount() const noexcept;
		VertexFormat GetVertexFormat() const noexcept;
		VertexBufferId GetVertexBufferId() const noexcept;
		IndexBufferId GetIndexBufferId() const noexcept;
		const BoundingBox& GetBoundingBox() const noexcept override;

		void Update(ID3D12GraphicsCommandList* commandList) const override;
		void Draw(ID3D12GraphicsCommandList* commandList, const Material* material) const override;

	private:
		Mesh() = delete;

		void CalculateBoundingBox(const void* verticesData, size_t verticesDataSize, VertexFormat _vertexFormat, BoundingBox& result);

		VertexBufferId vertexBufferId;
		IndexBufferId indexBufferId;

		uint32_t indicesCount;

		VertexFormat vertexFormat;
		BoundingBox boundingBox;

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		D3D12_INDEX_BUFFER_VIEW indexBufferView;

		ResourceManager& resourceManager = ResourceManager::GetInstance();
	};
}
