#include "GlobalConstants.hlsli"

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

StructuredBuffer<Particle> particleSystem : register(t0);
RWBuffer<uint> indexBuffer : register(u0);

[numthreads(1, 1, 1)]
void main(uint3 groupId : SV_GroupId)
{
	Particle particle = particleSystem[groupId.x];
	
	if (particle.currentLife > 0)
	{
		indexBuffer[groupId.x] = 0;
	};
}
