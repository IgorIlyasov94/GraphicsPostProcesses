cbuffer LocalConstBuffer : register(b0)
{
	float3 emitterPosition;
	uint emitterShape;

	float4 emitterVolume0;
	float4 emitterVolume1;
	
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
	float scatteringAngle;

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
	float3 padding;
};

struct ParticleBufferState
{
	uint particlesTotalCount;
	uint particlesPerBurstLeft;
	float particleGenerationRate;
	float particleGenerationTimer;
};

Texture1D<float4> sizeGradient : register(t0);
Texture1D<float4> velocityGradient : register(t1);
Texture1D<float4> colorGradient : register(t2);
RWStructuredBuffer<Particle> particleSystem : register(u0);
RWStructuredBuffer<ParticleBufferState> particleSystemState : register(u1);

[numthreads(1, 1, 1)]
void main(uint3 groupId : SV_GroupId)
{
	if (groupId.x == 0)
		particleSystemState[0].particlesPerBurstLeft = burstCount;
	
	Particle particle = particleSystem[groupId.x];
	
	if (particle.currentLife > 0)
	{
		
		
		particleSystem[groupId.x] = particle;
	};
}
