cbuffer CB : register( b0 )
{
	float3 position;
	float radius;
	float3 color;
	float intensity;
	uint pointLightNumber;
};

struct PointLight
{
	float3 position;
	float radius;
	float3 color;
	uint intensity;
};

RWStructuredBuffer<PointLight> pointLightBuffer : register( u0 );

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
	PointLight pointLight;
	pointLight.position = position;
	pointLight.radius = radius;
	pointLight.color = color;
	pointLight.intensity = intensity;
	
	pointLightBuffer[pointLightNumber] = pointLight;
}
