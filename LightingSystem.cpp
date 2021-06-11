#include "LightingSystem.h"
#include "Resources/Shaders/SetPointLightCS.hlsl.h"
#include "Resources/Shaders/DistributePointLightCS.hlsl.h"
#include "Resources/Shaders/CalculateClusterCoordinatesCS.hlsl.h"

Graphics::PointLightId Graphics::LightingSystem::CreatePointLight(float3 position, float3 color, float radius, float intensity, bool isShadowCaster)
{
	pointLights.push_back(PointLight(position, color, radius, intensity, isShadowCaster));

	return PointLightId(pointLights.size() - 1);
}

Graphics::PointLightId Graphics::LightingSystem::CreatePointLight(PointLight&& pointLight)
{
	pointLights.push_back(std::forward<PointLight>(pointLight));

	return PointLightId(pointLights.size() - 1);
}

Graphics::RWBufferId Graphics::LightingSystem::GetLightBufferId()
{
	return pointLightBufferId;
}

Graphics::RWTextureId Graphics::LightingSystem::GetLightClusterId()
{
	return pointLightClusterId;
}

void Graphics::LightingSystem::ComposeLightBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
	if (pointLights.empty())
		return;

	{
		float4 zero4{};
		std::vector<float4> initialClusterData(CLUSTER_SIZE * 2, zero4);

		clusterDataBufferId = resourceManager.CreateRWBuffer(initialClusterData.data(), CLUSTER_SIZE * sizeof(float4) * 2, sizeof(float4) * 2, CLUSTER_SIZE, DXGI_FORMAT_UNKNOWN);

		std::vector<PointLightBufferElement> pointLightBufferData(pointLights.size());

		for (uint32_t pointLightId = 0; pointLightId < pointLights.size(); pointLightId++)
		{
			pointLightBufferData[pointLightId].position = pointLights[pointLightId].GetPosition();
			pointLightBufferData[pointLightId].radius = pointLights[pointLightId].GetRadius();
			pointLightBufferData[pointLightId].color = pointLights[pointLightId].GetColor();
			pointLightBufferData[pointLightId].intensity = pointLights[pointLightId].GetIntensity();
		}

		pointLightBufferId = resourceManager.CreateRWBuffer(pointLightBufferData.data(), pointLightBufferData.size() * sizeof(PointLightBufferElement), sizeof(PointLightBufferElement),
			pointLightBufferData.size(), DXGI_FORMAT_UNKNOWN);

		TextureInfo pointLightClusterInfo{};
		pointLightClusterInfo.width = CLUSTER_SIZE_X * CLUSTER_LIGHTS_PER_CELL + 1;
		pointLightClusterInfo.height = CLUSTER_SIZE_Y;
		pointLightClusterInfo.depth = CLUSTER_SIZE_Z;
		pointLightClusterInfo.mipLevels = 1;
		pointLightClusterInfo.format = DXGI_FORMAT_R32_UINT;
		pointLightClusterInfo.dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		pointLightClusterInfo.srvDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
		
		pointLightClusterId = resourceManager.CreateRWTexture(pointLightClusterInfo);
	}
	
	{
		setPointLightCO = std::shared_ptr<ComputeObject>(new ComputeObject());
		setPointLightCO->AssignShader({ setPointLightCS, sizeof(setPointLightCS) });
		setPointLightCO->AssignRWBuffer(0, pointLightBufferId);

		pointLightConstBufferId = setPointLightCO->SetConstantBuffer(0, &pointLightConstBuffer, sizeof(pointLightConstBuffer));

		setPointLightCO->SetThreadGroupCount(1, 1, 1);
		setPointLightCO->Compose(device);

		calculateClusterCoordinatesCO = std::shared_ptr<ComputeObject>(new ComputeObject());
		calculateClusterCoordinatesCO->AssignShader({ calculateClusterCoordinatesCS, sizeof(calculateClusterCoordinatesCS) });
		calculateClusterCoordinatesCO->AssignRWBuffer(0, clusterDataBufferId);

		clusterDataConstBufferId = calculateClusterCoordinatesCO->SetConstantBuffer(0, &calculateClusterConstBuffer, sizeof(calculateClusterConstBuffer));

		calculateClusterCoordinatesCO->SetThreadGroupCount(18, 1, 1);
		calculateClusterCoordinatesCO->Compose(device);

		distributePointLightCO = std::shared_ptr<ComputeObject>(new ComputeObject());
		distributePointLightCO->AssignShader({ distributePointLightCS, sizeof(distributePointLightCS) });
		distributePointLightCO->AssignBuffer(0, clusterDataBufferId);
		distributePointLightCO->AssignBuffer(1, pointLightBufferId);
		distributePointLightCO->AssignRWTexture(0, pointLightClusterId);
		distributePointLightCO->SetThreadGroupCount(static_cast<uint32_t>(std::ceil(pointLights.size() / 16.0f)), 1, 1);
		distributePointLightCO->Compose(device);
	}
}

void Graphics::LightingSystem::SetPointLight(ID3D12GraphicsCommandList* commandList, const PointLightId& pointLightId, const PointLight& pointLight)
{
	pointLightConstBuffer.position = pointLight.GetPosition();
	pointLightConstBuffer.radius = pointLight.GetRadius();
	pointLightConstBuffer.color = pointLight.GetColor();
	pointLightConstBuffer.intensity = pointLight.GetIntensity();
	pointLightConstBuffer.pointLightId = pointLightId.value;

	setPointLightCO->UpdateConstantBuffer(pointLightConstBufferId, &pointLightConstBuffer, sizeof(pointLightConstBuffer));

	setPointLightCO->Present(commandList);
}

void Graphics::LightingSystem::SetPointLight(ID3D12GraphicsCommandList* commandList, const PointLightId& pointLightId, float3 position, float3 color, float radius, float intensity)
{
	pointLightConstBuffer.position = position;
	pointLightConstBuffer.radius = radius;
	pointLightConstBuffer.color = color;
	pointLightConstBuffer.intensity = intensity;
	pointLightConstBuffer.pointLightId = pointLightId.value;

	setPointLightCO->UpdateConstantBuffer(pointLightConstBufferId, &pointLightConstBuffer, sizeof(pointLightConstBuffer));

	setPointLightCO->Present(commandList);
}

void Graphics::LightingSystem::UpdateCluster(ID3D12GraphicsCommandList* commandList, const float4x4& invViewProjection)
{
	if (pointLights.empty())
		return;

	calculateClusterConstBuffer.invViewProjection = invViewProjection;
	calculateClusterCoordinatesCO->UpdateConstantBuffer(clusterDataConstBufferId, &calculateClusterConstBuffer, sizeof(calculateClusterConstBuffer));

	calculateClusterCoordinatesCO->Present(commandList);

	SetUAVBarrier(commandList, resourceManager.GetRWBuffer(pointLightBufferId).bufferAllocation.bufferResource);
	SetUAVBarrier(commandList, resourceManager.GetRWBuffer(clusterDataBufferId).bufferAllocation.bufferResource);

	distributePointLightCO->Present(commandList);

	SetUAVBarrier(commandList, resourceManager.GetRWTexture(pointLightClusterId).textureAllocation.textureResource);
}

void Graphics::LightingSystem::Clear()
{
	pointLights.clear();
}

Graphics::LightingSystem::LightingSystem()
	: calculateClusterConstBuffer{}, pointLightConstBuffer{}
{
	
}

Graphics::LightingSystem::~LightingSystem()
{

}
