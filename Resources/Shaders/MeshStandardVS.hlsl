#include "GlobalConstants.hlsli"

cbuffer LocalConstBuffer : register(b2)
{
	float4x4 world;
	float4x4 worldViewProj;
};

struct Input
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
};

struct Output
{
	float4 position : SV_Position;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD0;
	float4 clipCoord : TEXCOORD1;
	float4 worldCoord : TEXCOORD2;
};

Output main(Input input)
{
	Output output = (Output)0;
	
	output.position = mul(worldViewProj, float4(input.position, 1.0f));
	output.normal = normalize(mul((float3x3) world, input.normal).xyz);
	output.texCoord = input.texCoord;
	output.clipCoord = output.position;
	output.worldCoord = mul(world, float4(input.position, 1.0f));
	
	return output;
}
