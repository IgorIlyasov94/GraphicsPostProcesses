#include "GlobalConstants.hlsli"

cbuffer LocalConstBuffer : register(b2)
{
	float3 emitterPosition;
	uint emitterShape;

	float4 emitterVolume0;
	float3 emitterVolume1;
	bool isEmitterRelative;
	
	float burstPerSecond;
	uint burstCount;
	uint2 life;

	float2 angleStart;
	float2 angleSpeed;

	float2 sizeStart;
	float2 sizeEnd;

	float3 velocityStart;
	uint sizeAlterationType;
	
	float3 velocityEnd;
	float scatteringValue;

	uint velocityAlterationType;
	uint colorAlterationType;
	uint2 atlasSize;
	
	float4 colorStart;
	float4 colorEnd;
	
	uint2 frameXY;
	uint framesCount;
	uint particleMaxCount;
};

struct Particle
{
	float3 position;
	float angle;
	float2 size;
	float2 textureCoordOffset;
	float4 color;
	float3 velocity;
	uint life;
	uint currentLife;
	float angleSpeed;
	float2 padding;
};

struct ParticleBufferState
{
	int particlesTotalCount;
	int particlesPerBurstLeft;
	float particleGenerationRate;
	float particleGenerationTimer;
};

Texture1D<float4> sizeGradient : register(t0);
Texture1D<float4> velocityGradient : register(t1);
Texture1D<float4> colorGradient : register(t2);

SamplerState sLinear : register(s0);

RWStructuredBuffer<Particle> particleSystem : register(u0);
RWStructuredBuffer<ParticleBufferState> particleSystemState : register(u1);

float3 ParticleVelocity(float3 startVelocity, float3 endVelocity, float3 velocityDivergence, Texture1D<float4> gradient, SamplerState samplerState,
	float timeMoment, uint alterationType)
{
	float3 resultVelocity = 0.0f.xxx;
	
	if (alterationType == 0)
		resultVelocity = velocityDivergence;
	else
	{
		float3 resultDirection = startVelocity;
		float speed = 0.0f;
		
		if (alterationType == 1)
		{
			resultDirection = lerp(startVelocity, endVelocity, timeMoment);
			speed = length(resultDirection);
		}
		else
		{
			speed = length(resultDirection);
			resultDirection = gradient.SampleLevel(samplerState, timeMoment, 0.0f).xyz * 2.0f - 1.0f.xxx;
		}
		
		float3 divergencedDirection = resultDirection + velocityDivergence;
		float divergencedSpeed = length(divergencedDirection);
		
		if (speed > 0.0f)
			resultDirection = normalize((divergencedSpeed < 0.00001f) ? resultDirection : divergencedDirection);
		
		resultVelocity = speed * resultDirection;
	}
	
	return resultVelocity;
}

float2 ParticleSize(float2 startSize, float2 endSize, float2 currentSize, Texture1D<float4> gradient, SamplerState samplerState, float timeMoment,
	uint alterationType)
{
	float2 resultSize = currentSize;
	
	if (alterationType == 1)
		resultSize = lerp(startSize, endSize, timeMoment);
	else if (alterationType == 2)
		resultSize = startSize * gradient.SampleLevel(samplerState, timeMoment, 0.0f).xy;
	
	return resultSize;
}

float2 ParticleCoordOffset(uint2 startFrame, uint2 atlasCellsCount, uint animationFramesCount, float timeMoment)
{
	uint currentFrameX = startFrame.x + (uint) floor(animationFramesCount * timeMoment);
	uint currentFrameY = startFrame.y + currentFrameX / atlasCellsCount.x;
	currentFrameX %= atlasCellsCount.x;
	
	float2 resultOffset = uint2(currentFrameX, currentFrameY) / (float2) atlasCellsCount;
	
	return resultOffset;
}

float4 ParticleColor(float4 startColor, float4 endColor, float4 currentColor, Texture1D<float4> gradient, SamplerState samplerState, float timeMoment,
	uint alterationType)
{
	float4 resultColor = currentColor;
	
	if (alterationType == 1)
		resultColor = lerp(startColor, endColor, timeMoment);
	else if (alterationType == 2)
		resultColor = startColor * gradient.SampleLevel(samplerState, timeMoment, 0.0f);
	
	return resultColor;
}

float3 RandomPointInBox(float3 minCornerPoint, float3 maxCornerPoint, float3 randomValue)
{
	float3 result;
	
	for (int coordId = 0; coordId < 3; coordId++)
		result[coordId] = lerp(minCornerPoint[coordId], maxCornerPoint[coordId], randomValue[coordId]);
	
	return result;
}

float3 RandomPointInSphere(float3 center, float radius, float4 randomValue)
{
	float3 result = center + radius * randomValue.w * normalize(randomValue.xyz * 2.0f - 1.0f.xxx);
	
	return result;
}

float2 RandomSize(float2 minSize, float2 maxSize, float2 randomValue)
{
	float2 result;
	
	for (int coordId = 0; coordId < 2; coordId++)
		result[coordId] = lerp(minSize[coordId], maxSize[coordId], randomValue[coordId]);
	
	return result;
}

Particle GenerateNewParticle(float4 randomModifier)
{
	Particle particle = (Particle) 0;
	particle.position = (isEmitterRelative) ? 0.0f.xxx : emitterPosition;
	
	if (emitterShape == 1)
		particle.position += RandomPointInBox(emitterVolume0.xyz, emitterVolume1, frac(randomValues.xyz + randomModifier.xyz));
	else if (emitterShape == 2)
		particle.position += RandomPointInSphere(emitterVolume0.xyz, emitterVolume0.w, frac(randomValues + randomModifier));
	
	particle.angle = lerp(angleStart.x, angleStart.y, frac(randomValues.x + randomModifier.x));
	particle.size = ParticleSize(sizeStart, sizeEnd, RandomSize(sizeStart, sizeEnd, frac(randomValues.yz + randomModifier.xy)), sizeGradient, sLinear, 0.0f, sizeAlterationType);
	particle.textureCoordOffset = ParticleCoordOffset(frameXY, atlasSize, framesCount, 0.0f);
	particle.color = ParticleColor(colorStart, colorEnd, lerp(colorStart, colorEnd, frac(randomValues.w + randomModifier.x)), colorGradient, sLinear, 0.0f, colorAlterationType);
	particle.velocity = (velocityAlterationType == 0) ? RandomPointInBox(velocityStart, velocityEnd, frac(randomValues.zwy + randomModifier.xyz)) :
		RandomPointInSphere(0.0f.xxx, scatteringValue, frac(randomValues + randomModifier));
	particle.life = lerp(life.x, life.y, frac(randomValues.w + randomModifier.x));
	particle.currentLife = particle.life;
	particle.angleSpeed = lerp(angleSpeed.x, angleSpeed.y, frac(randomValues.y + randomModifier.x));
	
	return particle;
}

[numthreads(1, 1, 1)]
void main(uint3 groupId : SV_GroupId)
{
	if (groupId.x == 0)
	{
		particleSystemState[0].particleGenerationTimer += particleSystemState[0].particleGenerationRate;
		
		if (particleSystemState[0].particleGenerationTimer >= 1.0f)
		{
			particleSystemState[0].particlesPerBurstLeft = burstCount;
			particleSystemState[0].particleGenerationTimer = 0.0f;
		}
	}
	
	Particle particle = particleSystem[groupId.x];
	
	if (particle.currentLife > 0)
	{
		particle.currentLife--;
		
		if (particle.currentLife > 0)
		{
			float timeMoment = 1.0f - particle.currentLife / (float) particle.life;
			
			particle.position += ParticleVelocity(velocityStart, velocityEnd, particle.velocity, velocityGradient, sLinear, timeMoment,
				velocityAlterationType) * elapsedTime;
			particle.angle += particle.angleSpeed * elapsedTime;
			particle.size = ParticleSize(sizeStart, sizeEnd, particle.size, sizeGradient, sLinear, timeMoment, sizeAlterationType);
			particle.textureCoordOffset = ParticleCoordOffset(frameXY, atlasSize, framesCount, timeMoment);
			particle.color = ParticleColor(colorStart, colorEnd, particle.color, colorGradient, sLinear, timeMoment, colorAlterationType);
			
			particleSystem[groupId.x] = particle;
		}
		else
		{
			InterlockedAdd(particleSystemState[0].particlesTotalCount, -1);
			particleSystem[groupId.x].currentLife = 0;
		}
	}
	else
	{
		//if (particleSystemState[0].particlesPerBurstLeft > 0/* && particleSystemState[0].particlesTotalCount < particleMaxCount*/)
		{
			int particlesPerBurstLeft;
			InterlockedAdd(particleSystemState[0].particlesPerBurstLeft, -1, particlesPerBurstLeft);
			
			if (particlesPerBurstLeft > 0)
			{
				InterlockedAdd(particleSystemState[0].particlesTotalCount, 1u);
				
				particleSystem[groupId.x] = GenerateNewParticle(uint4(groupId.x % 2, groupId.x % 3, groupId.x % 4, groupId.x % 5) * randomValues.z * 2.0f - 1.0f.xxxx);
			}
		}
	}
}
