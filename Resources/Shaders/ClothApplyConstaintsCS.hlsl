cbuffer LocalConstBuffer : register(b0)
{
	float stiffness;
};

struct Joint
{
	uint vertexId0;
	uint vertexId1;
	float restLength;
	float padding;
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

Buffer<uint> jointIndexBuffer : register(t0);
StructuredBuffer<Joint> jointBuffer : register(t1);
RWStructuredBuffer<Vertex> vertexBuffer : register(u0);

[numthreads(1, 1, 1)]
void main(uint3 groupId : SV_GroupId)
{
	uint jointNumber = jointIndexBuffer[groupId.x * 2];
	uint jointStartIndex = jointIndexBuffer[groupId.x * 2 + 1];
	
	for (uint jointIndex = jointStartIndex; jointIndex < jointNumber; jointIndex++)
	{
		Joint joint = jointBuffer[jointIndex];
		Vertex vertex0 = vertexBuffer[joint.vertexId0];
		Vertex vertex1 = vertexBuffer[joint.vertexId1];
		
		float3 deltaPosition = vertex1.position - vertex0.position;
		float deltaPositionLength = max(length(deltaPosition), 1e-7);
		float stretching = 1.0f - joint.restLength / deltaPositionLength;
		deltaPosition *= stretching;
		
		float2 velocityCoeff = saturate(float2(vertex0.isFree - vertex1.isFree * 0.5f, vertex1.isFree - vertex0.isFree * 0.5f));
		
		vertex0.position += deltaPosition * velocityCoeff.x * stiffness;
		vertex1.position -= deltaPosition * velocityCoeff.y * stiffness;
		
		vertexBuffer[joint.vertexId0].position = vertex0.position;
		vertexBuffer[joint.vertexId1].position = vertex1.position;
	}
}
