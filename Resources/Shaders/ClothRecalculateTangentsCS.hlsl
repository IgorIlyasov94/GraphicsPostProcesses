static const uint JOINTS_PER_VERTEX_MAX_COUNT = 8;

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

struct AdjacentVertexIndices
{
	uint vertexIndex[JOINTS_PER_VERTEX_MAX_COUNT];
	uint jointsCount;
	float3 padding;
};

StructuredBuffer<AdjacentVertexIndices> adjacentVertexIndicesBuffer : register(t0);
RWStructuredBuffer<Vertex> vertexBuffer : register(u0);

float3 CalculateNormal(float3 tangentVector0, float3 tangentVector1)
{
	static const float EPSILON = 1e-7;
	
	float3 normal = cross(tangentVector0, tangentVector1);
    if (dot(normal, normal) < EPSILON)
        normal = float3(0.0f, 0.0f, 1.0f);
    normal = normalize(normal);
	
	return normal;
}

float3 CalculateTangent(float3 normal)
{
	static const float EPSILON = 1e-7;
	
	float3 testTangent0 = cross(normal, float3(0.0f, 0.0f, 1.0f));
	float3 testTangent1 = cross(normal, float3(0.0f, 1.0f, 0.0f));
	
	float3 tangent = (length(testTangent0) > length(testTangent1)) ? testTangent0 : testTangent1;
	if (dot(tangent, tangent) < EPSILON)
        tangent = float3(1.0f, 0.0f, 0.0f);
	tangent = normalize(tangent);
	
	return tangent;
}

float3 CalculateBinormal(float3 normal, float3 tangent)
{
	float3 binormal = cross(tangent, normal);
	binormal = normalize(binormal);
	
	return binormal;
}

[numthreads(1, 1, 1)]
void main(uint3 groupId : SV_GroupId)
{
	AdjacentVertexIndices adjacentVertexIndices = adjacentVertexIndicesBuffer[groupId.x];
	Vertex vertex0 = vertexBuffer[groupId.x];
	Vertex vertex1 = vertexBuffer[adjacentVertexIndices.vertexIndex[0]];
	
	float3 normal = 0.0f.xxx;
	float3 previousRawTangent = vertex1.position - vertex0.position;
	
	for (uint adjacentVertexIndexId = 1; adjacentVertexIndexId < adjacentVertexIndices.jointsCount; adjacentVertexIndexId++)
	{
		Vertex vertexN = vertexBuffer[adjacentVertexIndices.vertexIndex[adjacentVertexIndexId]];
		
		float3 rawTangent = vertexN.position - vertex0.position;
		
		normal += CalculateNormal(previousRawTangent, rawTangent);
		
		previousRawTangent = rawTangent;
	}
	
	vertex0.normal = normal / (float)(adjacentVertexIndices.jointsCount - 1);
	vertex0.tangent = CalculateTangent(vertex0.normal);
	vertex0.binormal = CalculateBinormal(vertex0.normal, vertex0.tangent);
	
	vertexBuffer[groupId.x] = vertex0;
}
