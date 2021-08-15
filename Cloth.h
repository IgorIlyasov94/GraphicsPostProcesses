#pragma once

#include "ResourceManager.h"
#include "Mesh.h"
#include "Material.h"
#include "ComputeObject.h"
#include "IRenderable.h"

namespace Graphics
{
	class Cloth final : public IRenderable
	{
	public:
		Cloth(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, std::filesystem::path meshFilePath, const std::vector<BoundingBox>& bindingBoxes,
			ConstantBufferId globalConstBufferId, float mass, float stiffness, float damping);
		~Cloth();

		const BoundingBox& GetBoundingBox() const noexcept override;
		RWBufferId GetVertexBuffer() const noexcept;

		void Update(ID3D12GraphicsCommandList* commandList) const override;
		void Draw(ID3D12GraphicsCommandList* commandList, const Material* material) const override;

	private:
		Cloth() = delete;

		RWBufferId ComposeVertexBuffer(const std::vector<std::shared_ptr<Vertex>>& primaryComposedVertices, const std::vector<BoundingBox>& bindingBoxes);
		size_t ComposeJointBuffers(const std::vector<std::shared_ptr<Vertex>>& primaryComposedVertices, const std::vector<uint32_t>& indices,
			BufferId& _adjacentVertexIndicesBufferId, BufferId& _jointInfoBufferId);

		struct JointInfo
		{
			uint32_t vertexIndex0;
			uint32_t vertexIndex1;
			float restLength;
			float padding;
		};

		static const size_t JOINTS_PER_VERTEX_MAX_COUNT = 8;

		struct AdjacentVertexIndices
		{
			uint32_t vertexIndex[JOINTS_PER_VERTEX_MAX_COUNT];
			uint32_t jointsCount;
			float3 padding;
		};

		void ComposeJointInfoBufferData(const std::vector<std::shared_ptr<Vertex>>& primaryComposedVertices, const std::vector<uint32_t>& indices,
			std::vector<JointInfo>& jointInfoBuffer);
		void ComposeAdjacentVertexIndicesBufferData(const std::vector<JointInfo>& jointInfoBuffer, size_t verticesCount,
			std::vector<AdjacentVertexIndices>& adjacentVertexIndicesBuffer);

		void SetupComputeObjects(ID3D12Device* device, ConstantBufferId globalConstBufferId, ConstantBufferId localConstBufferId, RWBufferId _vertexBufferId,
			size_t verticesCount, size_t jointsCount);

		RWBufferId vertexBufferId;
		IndexBufferId indexBufferId;
		BufferId jointInfoBufferId;
		BufferId adjacentVertexIndicesBufferId;

		uint32_t indicesCount;
		D3D12_INDEX_BUFFER_VIEW indexBufferView;
		
		std::unique_ptr<ComputeObject> clothApplyForcesCO;
		std::unique_ptr<ComputeObject> clothApplyConstraintsCO;
		std::unique_ptr<ComputeObject> clothApplyExternalConstraintsCO;
		std::unique_ptr<ComputeObject> clothApplySelfCollisionCO;
		std::unique_ptr<ComputeObject> clothRecalculateTangentsCO;

		ConstantBufferId localConstBufferId;

		struct LocalConstBuffer
		{
			float3 windStrength;
			float damping;
			float3 gravity;
			float mass;
			float stiffness;
			float previousElapsedTime;
			float2 padding;
		};

		BoundingBox boundingBox;

		LocalConstBuffer localConstBuffer;

		ResourceManager& resourceManager = ResourceManager::GetInstance();
	};
}
