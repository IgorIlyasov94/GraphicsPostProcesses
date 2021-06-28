cbuffer GlobalConstBuffer : register(b0)
{
	float4x4 view;
	float4x4 invView;
	float4x4 viewProjection;
	float4x4 invViewProjection;
	float3 cameraPosition;
	float padding;
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
