#pragma once

#include "PointLight.h"
#include "ComputeObject.h"
#include "ResourceManager.h"
#include "GraphicsSettings.h"

namespace Graphics
{
	template<uint8_t Category>
	class LightId
	{
	public:
		LightId()
			: value(0), category(Category)
		{

		}

		LightId(uint32_t resourceId)
			: value(resourceId), category(Category)
		{

		}

		uint32_t value;

	private:
		friend class LightingSystem;

		uint8_t category;
	};

	using PointLightId = typename LightId<0>;

	class LightingSystem
	{
	public:
		LightingSystem();
		~LightingSystem();

		PointLightId CreatePointLight(float3 position, float3 color, float radius, float intensity, bool isShadowCaster);
		PointLightId CreatePointLight(PointLight&& pointLight);

		RWBufferId GetLightBufferId();
		RWTextureId GetLightClusterId();

		void ComposeLightBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);

		void SetPointLight(ID3D12GraphicsCommandList* commandList, const PointLightId& pointLightId, const PointLight& pointLight);
		void SetPointLight(ID3D12GraphicsCommandList* commandList, const PointLightId& pointLightId, float3 position, float3 color, float radius, float intensity);
		
		void UpdateCluster(ID3D12GraphicsCommandList* commandList, const float4x4& invViewProjection);

		void Clear();

	private:
		std::vector<PointLight> pointLights;

		std::shared_ptr<ComputeObject> setPointLightCO;
		std::shared_ptr<ComputeObject> calculateClusterCoordinatesCO;
		std::shared_ptr<ComputeObject> distributePointLightCO;

		ConstantBufferId clusterDataConstBufferId;
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
		};

		using CalculateClusterConstBuffer = struct
		{
			float4x4 invViewProjection;
		};

		using PointLightBufferElement = struct
		{
			float3 position;
			float radius;
			float3 color;
			float intensity;
		};

		CalculateClusterConstBuffer calculateClusterConstBuffer;
		PointLightConstBuffer pointLightConstBuffer;

		static const uint32_t CLUSTER_SIZE_X = 8;
		static const uint32_t CLUSTER_SIZE_Y = 6;
		static const uint32_t CLUSTER_SIZE_Z = 6;
		static const uint32_t CLUSTER_LIGHTS_PER_CELL = 128;
		static const uint32_t CLUSTER_SIZE = CLUSTER_SIZE_X * CLUSTER_SIZE_Y * CLUSTER_SIZE_Z;

		ResourceManager& resourceManager = ResourceManager::GetInstance();
	};
}
