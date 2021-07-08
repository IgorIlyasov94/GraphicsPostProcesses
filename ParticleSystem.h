#pragma once

#include "GraphicsSettings.h"
#include "ResourceManager.h"
#include "Material.h"
#include "ComputeObject.h"

namespace Graphics
{
	enum class ParticleEmitterShape
	{
		EMITTER_POINT,
		EMITTER_BOX,
		EMITTER_SPHERE
	};

	enum class ParticleAnimationType
	{
		ANIMATION_NONE,
		ANIMATION_LERP,
		ANIMATION_GRADIENT
	};

	class ParticleSystem
	{
	public:
		ParticleSystem(uint32_t particlesMaxCount, uint32_t lifeMin, uint32_t lifeMax, TextureId paddingDefaultTexture);
		~ParticleSystem();

		void SetEmitter(float3 position, float burstPerSecond, uint32_t burstCount, bool particlesEmitterRelativeCoord, ParticleEmitterShape shape,
			std::variant<BoundingBox, BoundingSphere> volume);
		void SetAngularMotion(float angleMin, float angleMax, float angleSpeedMin, float angleSpeedMax);
		void SetSize(float2 sizeStart, float2 sizeEnd, ParticleAnimationType sizeAlterationType, TextureId _sizeGradientId);
		void SetVelocity(float3 velocityStart, float3 velocityEnd, float scatteringValue, ParticleAnimationType velocityAlterationType, TextureId _velocityGradientId);
		void SetColor(float4 colorStart, float4 colorEnd, ParticleAnimationType colorAlterationType, TextureId _colorGradientId);
		void SetFrames(uint32_t atlasRowNumber, uint32_t atlasColumnNumber, uint32_t frameRowId, uint32_t frameColumnId, uint32_t animationFramesCount);

		void Compose(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ConstantBufferId globalConstBufferId);

		RWBufferId GetParticleBufferId() const;
		ConstantBufferId GetParticleSystemConstBufferId() const;
		const BoundingBox& GetBoundingBox() const noexcept;
		const uint32_t& GetParticleMaxCount() const noexcept;

		const bool& IsComposed() const noexcept;

		void Update(ID3D12GraphicsCommandList* commandList) const;
		void Present(ID3D12GraphicsCommandList* commandList) const;

	private:
		ParticleSystem() = delete;

		using ParticleBuffer = struct
		{
			float3 position;
			float angle;
			float2 size;
			float2 textureCoordOffset;
			float4 color;
			float3 velocity;
			uint32_t life;
			uint32_t currentLife;
			float angleSpeed;
			float2 padding;
		};

		using ParticleBufferState = struct
		{
			uint32_t particlesTotalCount;
			uint32_t particlesPerBurstLeft;
			float particleGenerationRate;
			float particleGenerationTimer;
		};

		using ParticleSystemData = struct
		{
			float3 emitterPosition;
			uint32_t emitterShape;

			floatN emitterVolume0;

			float3 emitterVolume1;
			bool isEmitterRelative;

			float burstPerSecond;
			uint32_t burstCount;
			uint32_t lifeMin;
			uint32_t lifeMax;
			
			float2 angleStart;
			float2 angleSpeed;

			float2 sizeStart;
			float2 sizeEnd;

			float3 velocityStart;
			uint32_t sizeAlterationType;
			
			float3 velocityEnd;
			float scatteringValue;

			uint32_t velocityAlterationType;
			uint32_t colorAlterationType;
			uint32_t atlasSizeX;
			uint32_t atlasSizeY;

			float4 colorStart;
			float4 colorEnd;
			
			uint32_t frameX;
			uint32_t frameY;
			uint32_t framesCount;
			uint32_t particleMaxCount;
		};

		void CalculateBoundingBox(const ParticleSystemData& _particleSystemData, BoundingBox& result);

		ParticleSystemData particleSystemData;
		ConstantBufferId particleSystemConstBufferId;
		RWBufferId indexBufferId;
		RWBufferId particleBufferId;
		RWBufferId particleBufferStateId;
		std::shared_ptr<ComputeObject> sortParticleSystemCO;
		std::shared_ptr<ComputeObject> updateParticleSystemCO;

		BoundingBox boundingBox;

		TextureId sizeGradientId;
		TextureId velocityGradientId;
		TextureId colorGradientId;

		D3D12_INDEX_BUFFER_VIEW indexBufferView;
		
		bool isComposed;

		ResourceManager& resourceManager = ResourceManager::GetInstance();
	};
}
