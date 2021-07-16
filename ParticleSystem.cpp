#include "ParticleSystem.h"
#include "Resources/Shaders/UpdateParticleSystemCS.hlsl.h"
#include "Resources/Shaders/SortParticleSystemCS.hlsl.h"

Graphics::ParticleSystem::ParticleSystem(uint32_t particlesMaxCount, uint32_t lifeMin, uint32_t lifeMax, TextureId paddingDefaultTexture)
	: particleSystemData{}, boundingBox{}, indexBufferView{}, isComposed(false)
{
	particleSystemData.particleMaxCount = particlesMaxCount;
	particleSystemData.lifeMin = lifeMin;
	particleSystemData.lifeMax = lifeMax;
	particleSystemData.atlasSizeX = 1;
	particleSystemData.atlasSizeY = 1;
	particleSystemData.framesCount = 1;
	
	sizeGradientId = paddingDefaultTexture;
	velocityGradientId = paddingDefaultTexture;
	colorGradientId = paddingDefaultTexture;
}

Graphics::ParticleSystem::~ParticleSystem()
{

}

void Graphics::ParticleSystem::SetEmitter(float3 position, float burstPerSecond, uint32_t burstCount, bool particlesEmitterRelativeCoord, ParticleEmitterShape shape,
	std::variant<BoundingBox, BoundingSphere> volume)
{
	particleSystemData.emitterPosition = position;
	particleSystemData.burstPerSecond = burstPerSecond;
	particleSystemData.burstCount = burstCount;

	if (shape == ParticleEmitterShape::EMITTER_BOX)
	{
		particleSystemData.emitterShape = 1;

		auto& emitterVolume = std::get<BoundingBox>(volume);
		particleSystemData.emitterVolume0 = XMLoadFloat3(&emitterVolume.minCornerPoint);
		particleSystemData.emitterVolume1 = emitterVolume.maxCornerPoint;
	}
	else if (shape == ParticleEmitterShape::EMITTER_SPHERE)
	{
		particleSystemData.emitterShape = 2;

		auto& emitterVolume = std::get<BoundingSphere>(volume);
		particleSystemData.emitterVolume0 = XMLoadFloat3(&emitterVolume.center);
		particleSystemData.emitterVolume0.m128_f32[3] = emitterVolume.radius;
	}

	particleSystemData.isEmitterRelative = particlesEmitterRelativeCoord;
}

void Graphics::ParticleSystem::SetAngularMotion(float angleMin, float angleMax, float angleSpeedMin, float angleSpeedMax)
{
	particleSystemData.angleStart = { angleMin, angleMax };
	particleSystemData.angleSpeed = { angleSpeedMin, angleSpeedMax };
}

void Graphics::ParticleSystem::SetSize(float2 sizeStart, float2 sizeEnd, ParticleAnimationType sizeAlterationType, TextureId _sizeGradientId)
{
	particleSystemData.sizeStart = sizeStart;
	particleSystemData.sizeEnd = sizeEnd;

	if (sizeAlterationType != ParticleAnimationType::ANIMATION_NONE)
		particleSystemData.sizeAlterationType = (sizeAlterationType == ParticleAnimationType::ANIMATION_LERP) ? 1 : 2;

	sizeGradientId = _sizeGradientId;
}

void Graphics::ParticleSystem::SetVelocity(float3 velocityStart, float3 velocityEnd, float scatteringValue, ParticleAnimationType velocityAlterationType, TextureId _velocityGradientId)
{
	particleSystemData.velocityStart = velocityStart;
	particleSystemData.velocityEnd = velocityEnd;
	particleSystemData.scatteringValue = scatteringValue;

	if (velocityAlterationType != ParticleAnimationType::ANIMATION_NONE)
		particleSystemData.velocityAlterationType = (velocityAlterationType == ParticleAnimationType::ANIMATION_LERP) ? 1 : 2;

	velocityGradientId = _velocityGradientId;
}

void Graphics::ParticleSystem::SetColor(float4 colorStart, float4 colorEnd, ParticleAnimationType colorAlterationType, TextureId _colorGradientId)
{
	particleSystemData.colorStart = colorStart;
	particleSystemData.colorEnd = colorEnd;

	if (colorAlterationType != ParticleAnimationType::ANIMATION_NONE)
		particleSystemData.colorAlterationType = (colorAlterationType == ParticleAnimationType::ANIMATION_LERP) ? 1 : 2;

	colorGradientId = _colorGradientId;
}

void Graphics::ParticleSystem::SetFrames(uint32_t atlasRowNumber, uint32_t atlasColumnNumber, uint32_t frameRowId, uint32_t frameColumnId, uint32_t animationFramesCount)
{
	particleSystemData.atlasSizeX = atlasColumnNumber;
	particleSystemData.atlasSizeY = atlasRowNumber;
	particleSystemData.frameX = frameColumnId;
	particleSystemData.frameY = frameRowId;
	particleSystemData.framesCount = animationFramesCount;
}

void Graphics::ParticleSystem::Compose(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ConstantBufferId globalConstBufferId)
{
	particleSystemConstBufferId = resourceManager.CreateConstantBuffer(&particleSystemData, sizeof(particleSystemData));

	std::vector<ParticleBuffer> tempData(particleSystemData.particleMaxCount);
	std::fill(tempData.begin(), tempData.end(), ParticleBuffer{});
	
	particleBufferId = resourceManager.CreateRWBuffer(tempData.data(), tempData.size() * sizeof(ParticleBuffer), sizeof(ParticleBuffer),
		tempData.size(), DXGI_FORMAT_UNKNOWN, false);
	
	ParticleBufferState particleBufferStateData{};
	particleBufferStateData.particleGenerationTimer = 1.0f;
	particleBufferStateData.particleGenerationRate = particleSystemData.burstPerSecond / Graphics::GraphicsSettings::GetFramesPerSecond();

	particleBufferStateId = resourceManager.CreateRWBuffer(&particleBufferStateData, sizeof(particleBufferStateData), sizeof(particleBufferStateData),
		1, DXGI_FORMAT_UNKNOWN, false);

	std::vector<uint32_t> indexData(particleSystemData.particleMaxCount);
	std::iota(indexData.begin(), indexData.end(), 0);
	
	indexBufferId = resourceManager.CreateRWBuffer(indexData.data(), indexData.size() * sizeof(uint32_t), 0, indexData.size(), DXGI_FORMAT_R32_UINT, false);
	
	indexBufferView.BufferLocation = resourceManager.GetRWBuffer(indexBufferId).bufferAllocation.gpuAddress;
	indexBufferView.SizeInBytes = static_cast<UINT>(indexData.size() * sizeof(uint32_t));
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	resourceManager.SetResourceBarrier(commandList, indexBufferId, D3D12_RESOURCE_BARRIER_FLAG_NONE, D3D12_RESOURCE_STATE_INDEX_BUFFER);

	updateParticleSystemCO = std::shared_ptr<ComputeObject>(new ComputeObject());
	updateParticleSystemCO->AssignShader({ updateParticleSystemCS, sizeof(updateParticleSystemCS) });
	updateParticleSystemCO->AssignConstantBuffer(0, globalConstBufferId);
	updateParticleSystemCO->AssignConstantBuffer(1, particleSystemConstBufferId);
	updateParticleSystemCO->AssignTexture(0, sizeGradientId);
	updateParticleSystemCO->AssignTexture(1, velocityGradientId);
	updateParticleSystemCO->AssignTexture(2, colorGradientId);
	updateParticleSystemCO->SetSampler(0, D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 1);
	updateParticleSystemCO->AssignRWBuffer(0, particleBufferId);
	updateParticleSystemCO->AssignRWBuffer(1, particleBufferStateId);
	updateParticleSystemCO->SetThreadGroupCount(particleSystemData.particleMaxCount, 1, 1);
	updateParticleSystemCO->Compose(device);
	
	sortParticleSystemCO = std::shared_ptr<ComputeObject>(new ComputeObject());
	sortParticleSystemCO->AssignShader({ sortParticleSystemCS, sizeof(sortParticleSystemCS) });
	sortParticleSystemCO->AssignConstantBuffer(0, globalConstBufferId);
	sortParticleSystemCO->AssignBuffer(0, particleBufferId);
	sortParticleSystemCO->AssignRWBuffer(0, indexBufferId);
	sortParticleSystemCO->SetThreadGroupCount(particleSystemData.particleMaxCount, 1, 1);
	sortParticleSystemCO->Compose(device);

	CalculateBoundingBox(particleSystemData, boundingBox);

	isComposed = true;
}

Graphics::RWBufferId Graphics::ParticleSystem::GetParticleBufferId() const
{
	if (!isComposed)
		throw std::exception("Graphics::ParticleSystem::GetParticleBufferId: Particle System is not composed");

	return particleBufferId;
}

Graphics::ConstantBufferId Graphics::ParticleSystem::GetParticleSystemConstBufferId() const
{
	if (!isComposed)
		throw std::exception("Graphics::ParticleSystem::GetParticleSystemConstBufferId: Particle System is not composed");

	return particleSystemConstBufferId;
}

const Graphics::BoundingBox& Graphics::ParticleSystem::GetBoundingBox() const noexcept
{
	return boundingBox;
}

const uint32_t& Graphics::ParticleSystem::GetParticleMaxCount() const noexcept
{
	return particleSystemData.particleMaxCount;
}

const bool& Graphics::ParticleSystem::IsComposed() const noexcept
{
	return isComposed;
}

void Graphics::ParticleSystem::Update(ID3D12GraphicsCommandList* commandList) const
{
	std::array<D3D12_RESOURCE_BARRIER, 2> resourceBarriers{};
	resourceBarriers[0].Transition.pResource = resourceManager.GetRWBuffer(indexBufferId).bufferAllocation.bufferResource;
	resourceBarriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_INDEX_BUFFER;
	resourceBarriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	resourceBarriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	resourceBarriers[1].Transition.pResource = resourceManager.GetRWBuffer(particleBufferId).bufferAllocation.bufferResource;
	resourceBarriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	resourceBarriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	resourceBarriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	commandList->ResourceBarrier(static_cast<UINT>(resourceBarriers.size()), resourceBarriers.data());

	//sortParticleSystemCO->Present(commandList);
	updateParticleSystemCO->Present(commandList);
}

void Graphics::ParticleSystem::Draw(ID3D12GraphicsCommandList* commandList, const Material* material) const
{
	if (material != nullptr)
		if (material->IsComposed())
		{
			std::array<D3D12_RESOURCE_BARRIER, 4> resourceBarriers{};
			resourceBarriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
			resourceBarriers[0].UAV.pResource = resourceManager.GetRWBuffer(indexBufferId).bufferAllocation.bufferResource;

			resourceBarriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
			resourceBarriers[1].UAV.pResource = resourceManager.GetRWBuffer(particleBufferId).bufferAllocation.bufferResource;

			resourceBarriers[2].Transition.pResource = resourceBarriers[0].UAV.pResource;
			resourceBarriers[2].Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			resourceBarriers[2].Transition.StateAfter = D3D12_RESOURCE_STATE_INDEX_BUFFER;
			resourceBarriers[2].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			resourceBarriers[3].Transition.pResource = resourceBarriers[1].UAV.pResource;
			resourceBarriers[3].Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			resourceBarriers[3].Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
			resourceBarriers[3].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			commandList->ResourceBarrier(static_cast<UINT>(resourceBarriers.size()), resourceBarriers.data());

			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
			commandList->IASetIndexBuffer(&indexBufferView);

			material->Present(commandList);

			commandList->DrawIndexedInstanced(particleSystemData.particleMaxCount, 1, 0, 0, 0);
		}
}

void Graphics::ParticleSystem::CalculateBoundingBox(const ParticleSystemData& _particleSystemData, BoundingBox& result)
{
	floatN minPoint = { -100000.0f, -100000.0f, -100000.0f, 0.0f };
	floatN maxPoint = { 100000.0f, 100000.0f, 100000.0f, 0.0f };

	if (_particleSystemData.isEmitterRelative)
	{
		floatN velocityStart = XMLoadFloat3(&_particleSystemData.velocityStart);
		floatN velocityEnd = XMLoadFloat3(&_particleSystemData.velocityEnd);

		float sizeXMax = std::max(_particleSystemData.sizeStart.x, _particleSystemData.sizeEnd.x);
		float sizeYMax = std::max(_particleSystemData.sizeStart.y, _particleSystemData.sizeEnd.y);
		float sizeRadius = std::sqrt(sizeXMax * sizeXMax + sizeYMax * sizeYMax) / 2.0f;

		for (size_t coordId = 0; coordId < 3; coordId++)
		{
			minPoint.m128_f32[coordId] = std::min(velocityStart.m128_f32[coordId], velocityEnd.m128_f32[coordId]);
			minPoint.m128_f32[coordId] *= _particleSystemData.lifeMax / static_cast<float>(Graphics::GraphicsSettings::GetFramesPerSecond());
			minPoint.m128_f32[coordId] -= sizeRadius;
		}

		for (size_t coordId = 0; coordId < 3; coordId++)
		{
			maxPoint.m128_f32[coordId] = std::max(velocityStart.m128_f32[coordId], velocityEnd.m128_f32[coordId]);
			maxPoint.m128_f32[coordId] *= _particleSystemData.lifeMax / static_cast<float>(Graphics::GraphicsSettings::GetFramesPerSecond());
			maxPoint.m128_f32[coordId] += sizeRadius;
		}

		if (_particleSystemData.emitterShape == static_cast<uint32_t>(ParticleEmitterShape::EMITTER_BOX))
		{
			minPoint += _particleSystemData.emitterVolume0;
			maxPoint += XMLoadFloat3(&_particleSystemData.emitterVolume1);
		}
		else if (_particleSystemData.emitterShape == static_cast<uint32_t>(ParticleEmitterShape::EMITTER_SPHERE))
		{
			minPoint += floatN({ -1.0f, -1.0f, -1.0f, 0.0f }) * _particleSystemData.emitterVolume0.m128_f32[3];
			maxPoint += floatN({ 1.0f, 1.0f, 1.0f, 0.0f }) * _particleSystemData.emitterVolume0.m128_f32[3];
		}

		//<- HERE POTENTIALLY PHYSICS WILL HAVE AN IMPACT
	}

	XMStoreFloat3(&result.minCornerPoint, minPoint);
	XMStoreFloat3(&result.maxCornerPoint, maxPoint);
}
