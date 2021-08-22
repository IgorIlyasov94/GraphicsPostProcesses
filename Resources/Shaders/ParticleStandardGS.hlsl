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

StructuredBuffer<Particle> particleSystem : register(t0);

struct Input
{
	uint vertexId : VERTEXID;
};

struct Output
{
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD0;
	float4 color : COLOR;
};

static const float3 positions[4] =
{
	{ -1.0f, 1.0f, 0.0f },
	{ 1.0f, 1.0f, 0.0f },
	{ -1.0f, -1.0f, 0.0f },
	{ 1.0f, -1.0f, 0.0f }
};

static const float2 texCoords[4] =
{
	{ 0.0f, 0.0f },
	{ 1.0f, 0.0f },
	{ 0.0f, 1.0f },
	{ 1.0f, 1.0f }
};

float2 RotateVertex(float2 position, float angle)
{
	float sinAngle;
	float cosAngle;
	sincos(angle, sinAngle, cosAngle);
	
	float2x2 rotationMatrix =
	{
		cosAngle, -sinAngle,
		sinAngle, cosAngle
	};
	
	return mul(rotationMatrix, position);
}

[maxvertexcount(4)]
void main(point Input input[1], inout TriangleStream<Output> Stream)
{
	Output output = (Output)0;
	
	Particle particle = particleSystem[input[0].vertexId];
	
	if (particle.currentLife == 0)
		return;
	
	output.color = particle.color;
	
	[unroll]
	for (int particleVertexId = 0; particleVertexId < 4; particleVertexId++)
	{
		float3 position = positions[particleVertexId] * float3(particle.size, 1.0f);
		position.xy = RotateVertex(position.xy, particle.angle);
		position = mul((float3x3) invView, position).xyz;
		position += particle.position;
		
		if (isEmitterRelative)
			position += emitterPosition;
		
		output.position = mul(viewProjection, float4(position, 1.0f));
		output.texCoord = particle.textureCoordOffset + texCoords[particleVertexId] / (float2) atlasSize;
		Stream.Append(output);
	}
	
	Stream.RestartStrip();
}
