#pragma once

#include "PointLight.h"
#include "ComputeObject.h"
#include "ResourceManager.h"
#include "GraphicsSettings.h"

namespace Graphics
{
	using PointLightId = typename ResourceId<20>;

	class LightingSystem
	{
	public:
		LightingSystem();
		~LightingSystem();

		PointLightId CreatePointLight(float3 position, float3 color, float radius, float intensity, bool isShadowCaster);
		PointLightId CreatePointLight(PointLight&& pointLight);

		RWBufferId GetLightBufferId() const;
		RWTextureId GetLightClusterId() const;

		RWBufferId GetClusterId() const;

		void ComposeLightBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ConstantBufferId _immutableGlobalConstBufferId, ConstantBufferId _globalConstBufferId);

		void SetPointLight(ID3D12GraphicsCommandList* commandList, const PointLightId& pointLightId, const PointLight& pointLight);
		void SetPointLight(ID3D12GraphicsCommandList* commandList, const PointLightId& pointLightId, float3 position, float3 color, float radius, float intensity);
		
		void UpdateCluster(ID3D12GraphicsCommandList* commandList);

		void Clear();

	private:
		std::vector<PointLight> pointLights;

		std::shared_ptr<ComputeObject> setPointLightCO;
		std::shared_ptr<ComputeObject> calculateClusterCoordinatesCO;
		std::shared_ptr<ComputeObject> distributePointLightCO;

		ConstantBufferId immutableGlobalConstBufferId;
		ConstantBufferId globalConstBufferId;

		ConstantBufferId pointLightConstBufferId;
		RWBufferId clusterDataBufferId;
		RWBufferId pointLightBufferId;
		RWTextureId pointLightClusterId;

		RenderTargetId sceneRenderTargetId;

		using PointLightConstBuffer = struct
		{
			float3 position;
			float radius;
			float3 color;
			float intensity;
			uint32_t pointLightId;
			float3 padding;
		};

		using PointLightBufferElement = struct
		{
			float3 position;
			float radius;
			float3 color;
			float intensity;
		};

		PointLightConstBuffer pointLightConstBuffer;

		static const uint32_t CLUSTER_SIZE_X = 8;
		static const uint32_t CLUSTER_SIZE_Y = 6;
		static const uint32_t CLUSTER_SIZE_Z = 6;
		static const uint32_t CLUSTER_LIGHTS_PER_CELL = 128;
		static const uint32_t CLUSTER_SIZE = CLUSTER_SIZE_X * CLUSTER_SIZE_Y * CLUSTER_SIZE_Z;

		ResourceManager& resourceManager = ResourceManager::GetInstance();
	};
}
