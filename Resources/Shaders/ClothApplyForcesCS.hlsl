cbuffer GlobalConstBuffer : register(b0)
{
	float4x4 view;
	float4x4 invView;
	float4x4 viewProjection;
	float4x4 invViewProjection;
	float3 cameraPosition;
	float elapsedTime;
	float4 randomValues;
};

cbuffer LocalConstBuffer : register(b1)
{
	float3 windStrength;
	float stiffness;
	float3 gravity;
	float mass;
	float damping;
	float previousElapsedTime;
	float2 padding;
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
};

RWStructuredBuffer<Vertex> vertexBuffer : register(u0);

[numthreads(1, 1, 1)]
void main(uint3 groupId : SV_GroupId)
{
	Vertex vertex = vertexBuffer[groupId.x];
	
	if (vertex.isFree == 1)
	{
		float3 newPosition = vertex.position;
		float3 forces = dot(windStrength, vertex.normal) * vertex.normal;
		forces += gravity * mass;
		
		newPosition += (elapsedTime / previousElapsedTime) * damping * (vertex.position - vertex.previousPosition) + forces * elapsedTime * previousElapsedTime;
		
		vertexBuffer[groupId.x].position = newPosition;
		vertexBuffer[groupId.x].previousPosition = vertex.position;
	}
}
