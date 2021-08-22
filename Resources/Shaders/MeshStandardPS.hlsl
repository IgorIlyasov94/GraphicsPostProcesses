#include "GlobalConstants.hlsli"

cbuffer LocalConstBuffer : register(b2)
{
	float4x4 world;
	float4x4 worldViewProj;
};

struct PointLight
{
	float3 position;
	float radius;
	float3 color;
	float intensity;
};

StructuredBuffer<PointLight> pointLightBuffer : register(t0);
Texture3D<uint> pointLightCluster : register(t1);

struct Input
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD0;
	float4 clipCoord : TEXCOORD1;
	float4 worldCoord : TEXCOORD2;
};

struct Output
{
	float4 color : SV_TARGET0;
	float4 normal : SV_TARGET1;
};

static const uint CLUSTER_SIZE_X = 8;
static const uint CLUSTER_SIZE_Y = 6;
static const uint CLUSTER_SIZE_Z = 6;
static const uint CLUSTER_SIZE = CLUSTER_SIZE_X * CLUSTER_SIZE_Y * CLUSTER_SIZE_Z;
static const uint3 CLUSTER_SIZE_XYZ = uint3(CLUSTER_SIZE_X, CLUSTER_SIZE_Y, CLUSTER_SIZE_Z);
static const uint CLUSTER_LIGHTS_PER_CELL = 128;

uint3 Calculate3DAddress(float3 screenCoord, uint3 clusterSize)
{
	uint3 resultAddress;
	resultAddress.xy = (uint2) (screenCoord.xy * clusterSize.xy);
	resultAddress.z = (uint) (screenCoord.z * clusterSize.z);
	return resultAddress;
}

float LightAttenuation(float lightDistance, float lightRadius)
{
	return 1.0f - smoothstep(lightRadius * 0.75f, lightRadius, lightDistance);
}

float3 CalculateLighting(PointLight pointLight, float3 position, float3 normal)
{
	float3 lightDirection = position - pointLight.position;
	float lightDistance = length(lightDirection);
	lightDirection = lightDirection / lightDistance;
	
	float brightness = saturate(dot(lightDirection, -normal)) * LightAttenuation(lightDistance, pointLight.radius);
	
	return pointLight.color * pointLight.intensity * brightness;
}

float LinearizeZ(float nonLinearZ)
{
	float zNear = 0.01f;
	float zFar = 1024.0f;
	
	float2 linearizeCoeff = float2((1.0f - zFar / zNear) / zFar, 1.0f / zNear);
	
	return 1.0f / (nonLinearZ * linearizeCoeff.x + linearizeCoeff.y);
}

float FromNearFarTo01(float z)
{
	float zNear = 0.01f;
	float zFar = 1024.0f;
	
	return (z - zNear) / (zFar - zNear);
}

[earlydepthstencil]
Output main(Input input)
{
	Output output = (Output)0;
	
	float2 xyValues = input.position.xy / float2(1920.0f, 1080.0f);//saturate((input.clipCoord.xy / input.clipCoord.w) * 0.5f + 1.0f.xx);
	float zValue = FromNearFarTo01(LinearizeZ(input.clipCoord.z / input.clipCoord.w));
	
	uint3 address = Calculate3DAddress(float3(xyValues, zValue), CLUSTER_SIZE_XYZ);
	address = clamp(address, uint3(0, 0, 0), CLUSTER_SIZE_XYZ - uint3(1, 1, 1));
	address.x *= CLUSTER_LIGHTS_PER_CELL;
	
	uint lightCount = pointLightCluster[address];
	
	float3 diffuse = 0.1f.xxx;
	
	for (uint lightId = 1; lightId <= lightCount; lightId++)
	{
		uint lightAddress = pointLightCluster[address + uint3(lightId, 0, 0)];
		
		diffuse += CalculateLighting(pointLightBuffer[lightAddress], input.worldCoord.xyz, input.normal);
	}
	
	output.color = float4(diffuse, 1.0f);
	
	output.normal = float4(input.normal, 1.0f);
	
	return output;
}
