cbuffer LocalConstBuffer : register(b0)
{
	float3 windStrength;
	float damping;
	float3 gravity;
	float mass;
	float stiffness;
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
	float2 padding;
};

struct JointInfo
{
	uint vertexIndex0;
	uint vertexIndex1;
	float restLength;
	float padding;
};

StructuredBuffer<JointInfo> jointInfoBuffer : register(t0);
RWStructuredBuffer<Vertex> vertexBuffer : register(u0);

[numthreads(1, 1, 1)]
void main(uint3 groupId : SV_GroupId)
{
	JointInfo jointInfo = jointInfoBuffer[groupId.x];
	
	Vertex vertex0 = vertexBuffer[jointInfo.vertexIndex0];
	Vertex vertex1 = vertexBuffer[jointInfo.vertexIndex1];
	
	if (vertex0.isFree == 0 && vertex1.isFree == 0)
		return;
	
	float3 deltaPosition = vertex1.position - vertex0.position;
	float deltaPositionLength = max(length(deltaPosition), 1e-7);
	float stretching = 1.0f - jointInfo.restLength / deltaPositionLength;
	deltaPosition *= stretching;
	
	float2 velocityCoeff = saturate(float2(vertex0.isFree - vertex1.isFree * 0.5f, vertex1.isFree - vertex0.isFree * 0.5f));
	
	if (vertex0.isFree == 1)
	{
		vertex0.position += deltaPosition * velocityCoeff.x * stiffness;
		vertexBuffer[jointInfo.vertexIndex0].position = vertex0.position;
	}
	
	if (vertex1.isFree == 1)
	{
		vertex1.position -= deltaPosition * velocityCoeff.y * stiffness;
		vertexBuffer[jointInfo.vertexIndex1].position = vertex1.position;
	}
}
