#include "GlobalConstants.hlsli"

cbuffer LocalConstBuffer : register(b2)
{
	float4x4 world;
	float4x4 worldViewProj;
};

struct Vertex
{
	float3 position;
	float3 previousPosition;
	float3 normal;
	float3 tangent;
	float3 binormal;
	float2 texCoord;
	uint isFree;
	float2 padding;
};

StructuredBuffer<Vertex> vertexBuffer : register(t2);

struct Input
{
	uint vertexId : SV_VertexId;
};

struct Output
{
	float4 position : SV_Position;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD0;
	float4 clipCoord : TEXCOORD1;
	float4 worldCoord : TEXCOORD2;
	float free : TEXCOORD3;
};

Output main(Input input)
{
	Output output = (Output)0;
	
	output.position = mul(worldViewProj, float4(vertexBuffer[input.vertexId].position, 1.0f));
	output.normal = normalize(mul((float3x3) world, vertexBuffer[input.vertexId].normal).xyz);
	output.texCoord = vertexBuffer[input.vertexId].texCoord;
	output.clipCoord = output.position;
	output.worldCoord = mul(world, float4(vertexBuffer[input.vertexId].position, 1.0f));
	output.free = (float) vertexBuffer[input.vertexId].isFree;
	
	return output;
}
