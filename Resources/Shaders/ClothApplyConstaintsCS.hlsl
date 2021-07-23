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

struct JointInfo
{
	uint adjacentPointsNumber;
	uint adjacentPointIndexOffset;
	float restLength;
	float stiffness;
};

StructuredBuffer<JointInfo> jointInfoBuffer : register(t0);
Buffer<uint> jointBuffer : register(t1);
RWStructuredBuffer<Vertex> vertexBuffer : register(u0);

[numthreads(1, 1, 1)]
void main(uint3 groupId : SV_GroupId)
{
	JointInfo jointInfo = jointInfoBuffer[groupId.x];
	
	uint vertexId0 = groupId.x;
	Vertex vertex0 = vertexBuffer[vertexId0];
	
	for (uint adjacentPointIndex = 0; adjacentPointIndex < jointInfo.adjacentPointsNumber; adjacentPointIndex++)
	{
		if (vertex0.isFree == 1)
		{
			uint vertexId1 = jointBuffer[adjacentPointIndex + jointInfo.adjacentPointIndexOffset];
			Vertex vertex1 = vertexBuffer[vertexId1];
			
			float3 deltaPosition = vertex1.position - vertex0.position;
			float deltaPositionLength = max(length(deltaPosition), 1e-7);
			float stretching = 1.0f - jointInfo.restLength / deltaPositionLength;
			deltaPosition *= stretching;
			
			float2 velocityCoeff = saturate(float2(vertex0.isFree - vertex1.isFree * 0.5f, vertex1.isFree - vertex0.isFree * 0.5f));
			
			vertex0.position += deltaPosition * velocityCoeff.x * jointInfo.stiffness;
		}
	}
		
	if (vertex0.isFree == 1)
		vertexBuffer[vertexId0].position = vertex0.position;
}
