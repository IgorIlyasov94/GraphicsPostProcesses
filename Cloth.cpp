#include "Cloth.h"
#include "GeometryProcessor.h"
#include "MeshProcessor.h"
#include "IMeshLoader.h"
#include "OBJLoader.h"
#include "Resources/Shaders/ClothApplyForcesCS.hlsl.h"
#include "Resources/Shaders/ClothApplyExternalConstraintsCS.hlsl.h"
#include "Resources/Shaders/ClothApplyConstaintsCS.hlsl.h"
#include "Resources/Shaders/ClothApplySelfCollisionCS.hlsl.h"
#include "Resources/Shaders/ClothRecalculateTangentsCS.hlsl.h"

Graphics::Cloth::Cloth(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, std::filesystem::path meshFilePath, const std::vector<BoundingBox>& bindingBoxes,
	ConstantBufferId globalConstBufferId, float mass, float stiffness, float damping)
	: boundingBox{}, localConstBuffer{}
{
	std::unique_ptr<IMeshLoader> meshLoader;

	if (meshFilePath.extension() == ".obj" || meshFilePath.extension() == ".OBJ")
		meshLoader = std::make_unique<Graphics::OBJLoader>();
	else
		throw std::exception("Mesh::Mesh: Unsupported file extension");

	SplittedMeshData splittedMeshData;
	meshLoader->Load(meshFilePath, splittedMeshData);

	MeshProcessor meshProcessor(splittedMeshData);
	meshProcessor.CalculateNormals(true);
	meshProcessor.CalculateTangents();
	meshProcessor.ConvertPolygons(PolygonFormat::QUAD);

	VertexFormat resultVertexFormat{};
	meshProcessor.Compose(VertexFormat::POSITION | VertexFormat::NORMAL | VertexFormat::TANGENT_BINORMAL | VertexFormat::TEXCOORD, true, resultVertexFormat);

	boundingBox = meshProcessor.GetBoundingBox();

	auto boundingBoxSize = BoundingBoxSize(boundingBox);

	XMStoreFloat3(&boundingBox.minCornerPoint, XMLoadFloat3(&boundingBox.minCornerPoint) - XMLoadFloat3(&boundingBoxSize));
	XMStoreFloat3(&boundingBox.maxCornerPoint, XMLoadFloat3(&boundingBox.maxCornerPoint) + XMLoadFloat3(&boundingBoxSize));

	std::vector<std::shared_ptr<Vertex>> primaryComposedVertices;
	std::vector<uint32_t> composedIndices;
	meshProcessor.GetComposedData(primaryComposedVertices, composedIndices);
	size_t verticesCount = primaryComposedVertices.size();
	indicesCount = composedIndices.size();

	vertexBufferId = ComposeVertexBuffer(primaryComposedVertices, bindingBoxes);
	indexBufferId = resourceManager.CreateIndexBuffer(composedIndices.data(), composedIndices.size() * sizeof(uint32_t), sizeof(uint32_t));
	indexBufferView = resourceManager.GetIndexBufferView(indexBufferId);

	size_t jointsCount = ComposeJointBuffers(primaryComposedVertices, composedIndices, adjacentVertexIndicesBufferId, jointInfoBufferId);

	localConstBuffer.damping = damping;
	localConstBuffer.gravity = float3(0.0f, -9.780318f, 0.0f);
	localConstBuffer.mass = mass;
	localConstBuffer.stiffness = stiffness;

	localConstBufferId = resourceManager.CreateConstantBuffer(&localConstBuffer, sizeof(localConstBuffer));

	SetupComputeObjects(device, globalConstBufferId, localConstBufferId, vertexBufferId, verticesCount, jointsCount);
}

Graphics::Cloth::~Cloth()
{

}

const Graphics::BoundingBox& Graphics::Cloth::GetBoundingBox() const noexcept
{
	return boundingBox;
}

Graphics::RWBufferId Graphics::Cloth::GetVertexBuffer() const noexcept
{
	return vertexBufferId;
}

void Graphics::Cloth::Update(ID3D12GraphicsCommandList* commandList) const
{
	resourceManager.SetResourceBarrier(commandList, vertexBufferId, D3D12_RESOURCE_BARRIER_FLAG_NONE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	//clothApplyForcesCO->Present(commandList);
	//clothApplyConstraintsCO->Present(commandList);
	//clothApplyExternalConstraintsCO->Present(commandList); waiting for physics
	//clothApplySelfCollisionCO->Present(commandList); waiting for someday
	//clothRecalculateTangentsCO->Present(commandList);

	resourceManager.SetResourceBarrier(commandList, vertexBufferId, D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

void Graphics::Cloth::Draw(ID3D12GraphicsCommandList* commandList, const Material* material) const
{
	resourceManager.SetUAVBarrier(commandList, vertexBufferId);
	resourceManager.SetResourceBarrier(commandList, vertexBufferId, D3D12_RESOURCE_BARRIER_FLAG_END_ONLY, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	if (material != nullptr)
		if (material->IsComposed())
		{
			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			commandList->IASetIndexBuffer(&indexBufferView);

			material->Present(commandList);

			commandList->DrawIndexedInstanced(indicesCount, 1, 0, 0, 0);
		}
}

Graphics::RWBufferId Graphics::Cloth::ComposeVertexBuffer(const std::vector<std::shared_ptr<Vertex>>& primaryComposedVertices, const std::vector<BoundingBox>& bindingBoxes)
{
	struct ClothVertex
	{
		float3 position;
		float3 previousPosition;
		float3 normal;
		float3 tangent;
		float3 binormal;
		float2 texCoord;
		uint32_t isFree;
		float2 padding;
	};

	std::vector<ClothVertex> vertices;
	vertices.reserve(primaryComposedVertices.size());
	
	for (auto& vertex : primaryComposedVertices)
	{
		ClothVertex clothVertex{};
		clothVertex.position = vertex->position;
		clothVertex.previousPosition = clothVertex.position;
		clothVertex.normal = vertex->normal;
		clothVertex.tangent = vertex->tangent;
		clothVertex.binormal = vertex->binormal;
		clothVertex.texCoord = vertex->texCoord;
		clothVertex.isFree = 1;

		for (auto& bindingBox : bindingBoxes)
			if (CheckPointInBox(clothVertex.position, bindingBox))
			{
				clothVertex.isFree = 0;

				break;
			}

		vertices.push_back(clothVertex);
	}

	return resourceManager.CreateRWBuffer(vertices.data(), vertices.size() * sizeof(ClothVertex), sizeof(ClothVertex), vertices.size(), DXGI_FORMAT_UNKNOWN, false);
}

size_t Graphics::Cloth::ComposeJointBuffers(const std::vector<std::shared_ptr<Vertex>>& primaryComposedVertices, const std::vector<uint32_t>& indices,
	BufferId& _adjacentVertexIndicesBufferId, BufferId& _jointInfoBufferId)
{
	std::vector<JointInfo> jointInfoBufferData;
	ComposeJointInfoBufferData(primaryComposedVertices, indices, jointInfoBufferData);
	_jointInfoBufferId = resourceManager.CreateBuffer(jointInfoBufferData.data(), jointInfoBufferData.size() * sizeof(JointInfo), sizeof(JointInfo),
		jointInfoBufferData.size(), DXGI_FORMAT_UNKNOWN);

	std::vector<AdjacentVertexIndices> adjacentVertexIndicesBufferData;
	ComposeAdjacentVertexIndicesBufferData(jointInfoBufferData, indices.size(), adjacentVertexIndicesBufferData);
	_adjacentVertexIndicesBufferId = resourceManager.CreateBuffer(adjacentVertexIndicesBufferData.data(), adjacentVertexIndicesBufferData.size() * sizeof(AdjacentVertexIndices),
		sizeof(AdjacentVertexIndices), adjacentVertexIndicesBufferData.size(), DXGI_FORMAT_UNKNOWN);

	return jointInfoBufferData.size();
}

void Graphics::Cloth::ComposeJointInfoBufferData(const std::vector<std::shared_ptr<Vertex>>& primaryComposedVertices, const std::vector<uint32_t>& indices,
	std::vector<JointInfo>& jointInfoBuffer)
{
	jointInfoBuffer.clear();
	jointInfoBuffer.reserve(primaryComposedVertices.size() * 6 / 4);

	for (size_t currentIndexId = 0; currentIndexId < indices.size(); currentIndexId += 4)
	{
		auto composeJointInfo = [&currentIndexId, &primaryComposedVertices, &indices](const size_t& vertexIndexId0, const size_t& vertexIndexId1)
		{
			JointInfo jointInfo{};
			jointInfo.vertexIndex0 = indices[currentIndexId + vertexIndexId0];
			jointInfo.vertexIndex1 = indices[currentIndexId + vertexIndexId1];
			jointInfo.restLength = XMVector3Length(XMLoadFloat3(&primaryComposedVertices[jointInfo.vertexIndex0]->position) -
				XMLoadFloat3(&primaryComposedVertices[jointInfo.vertexIndex1]->position)).m128_f32[0];

			return jointInfo;
		};

		jointInfoBuffer.push_back(composeJointInfo(0, 3));
		jointInfoBuffer.push_back(composeJointInfo(1, 2));

		auto compareJoints = [](const JointInfo& jointInfo0)
		{
			return [&jointInfo0](const JointInfo& jointInfo1)
			{
				return jointInfo0.vertexIndex0 == jointInfo1.vertexIndex0 && jointInfo0.vertexIndex1 == jointInfo1.vertexIndex1 ||
					jointInfo0.vertexIndex0 == jointInfo1.vertexIndex1 && jointInfo0.vertexIndex1 == jointInfo1.vertexIndex0;
			};
		};

		JointInfo currentJointInfo = composeJointInfo(0, 1);
		if (std::find_if(jointInfoBuffer.begin(), jointInfoBuffer.end(), compareJoints(currentJointInfo)) == jointInfoBuffer.end())
			jointInfoBuffer.push_back(currentJointInfo);

		currentJointInfo = composeJointInfo(0, 2);
		if (std::find_if(jointInfoBuffer.begin(), jointInfoBuffer.end(), compareJoints(currentJointInfo)) == jointInfoBuffer.end())
			jointInfoBuffer.push_back(currentJointInfo);

		currentJointInfo = composeJointInfo(1, 2);
		if (std::find_if(jointInfoBuffer.begin(), jointInfoBuffer.end(), compareJoints(currentJointInfo)) == jointInfoBuffer.end())
			jointInfoBuffer.push_back(currentJointInfo);

		currentJointInfo = composeJointInfo(2, 3);
		if (std::find_if(jointInfoBuffer.begin(), jointInfoBuffer.end(), compareJoints(currentJointInfo)) == jointInfoBuffer.end())
			jointInfoBuffer.push_back(currentJointInfo);
	}

	jointInfoBuffer.shrink_to_fit();
}

void Graphics::Cloth::ComposeAdjacentVertexIndicesBufferData(const std::vector<JointInfo>& jointInfoBuffer, size_t verticesCount,
	std::vector<AdjacentVertexIndices>& adjacentVertexIndicesBuffer)
{
	adjacentVertexIndicesBuffer.clear();
	adjacentVertexIndicesBuffer.reserve(verticesCount * 8);

	for (uint32_t vertexIndex = 0; vertexIndex < verticesCount; vertexIndex++)
	{
		AdjacentVertexIndices adjacentVertexIndices{};

		for (auto& jointInfo : jointInfoBuffer)
		{
			if (jointInfo.vertexIndex0 == vertexIndex)
				adjacentVertexIndices.vertexIndex[adjacentVertexIndices.jointsCount++] = jointInfo.vertexIndex1;
			else if(jointInfo.vertexIndex1 == vertexIndex)
				adjacentVertexIndices.vertexIndex[adjacentVertexIndices.jointsCount++] = jointInfo.vertexIndex0;

			if (adjacentVertexIndices.jointsCount == JOINTS_PER_VERTEX_MAX_COUNT)
				break;
		}

		adjacentVertexIndicesBuffer.push_back(adjacentVertexIndices);
	}

	adjacentVertexIndicesBuffer.shrink_to_fit();
}

void Graphics::Cloth::SetupComputeObjects(ID3D12Device* device, ConstantBufferId globalConstBufferId, ConstantBufferId localConstBufferId,
	RWBufferId _vertexBufferId, size_t verticesCount, size_t jointsCount)
{
	clothApplyForcesCO = std::unique_ptr<ComputeObject>(new ComputeObject());
	clothApplyForcesCO->AssignShader({ clothApplyForcesCS, sizeof(clothApplyForcesCS) });
	clothApplyForcesCO->AssignConstantBuffer(0, globalConstBufferId);
	clothApplyForcesCO->AssignConstantBuffer(1, localConstBufferId);
	clothApplyForcesCO->AssignRWBuffer(0, _vertexBufferId);
	clothApplyForcesCO->SetThreadGroupCount(verticesCount, 1, 1);
	clothApplyForcesCO->Compose(device);

	clothApplyConstraintsCO = std::unique_ptr<ComputeObject>(new ComputeObject());
	clothApplyConstraintsCO->AssignShader({ clothApplyConstaintsCS, sizeof(clothApplyConstaintsCS) });
	clothApplyConstraintsCO->AssignConstantBuffer(0, localConstBufferId);
	clothApplyConstraintsCO->AssignBuffer(0, jointInfoBufferId);
	clothApplyConstraintsCO->AssignRWBuffer(0, _vertexBufferId);
	clothApplyConstraintsCO->SetThreadGroupCount(jointsCount, 1, 1);
	clothApplyConstraintsCO->Compose(device);

	/*clothApplyExternalConstraintsCO = std::unique_ptr<ComputeObject>(new ComputeObject());
	clothApplyExternalConstraintsCO->AssignShader({ clothApplyExternalConstraintsCS, sizeof(clothApplyExternalConstraintsCS) });
	clothApplyExternalConstraintsCO-> physics required*/

	/*clothApplySelfCollisionCO later maybe...*/

	clothRecalculateTangentsCO = std::unique_ptr<ComputeObject>(new ComputeObject());
	clothRecalculateTangentsCO->AssignShader({ clothRecalculateTangentsCS, sizeof(clothRecalculateTangentsCS) });
	clothRecalculateTangentsCO->AssignBuffer(0, adjacentVertexIndicesBufferId);
	clothRecalculateTangentsCO->AssignRWBuffer(0, _vertexBufferId);
	clothRecalculateTangentsCO->SetThreadGroupCount(verticesCount, 1, 1);
	clothRecalculateTangentsCO->Compose(device);
}
