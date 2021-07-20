struct Input
{
	uint vertexId : VERTEXID;
};

struct Output
{
    
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

static const float EPSILON = 1e-7;

[maxvertexcount(1)]
void main(triangle Input input[3], inout PointStream<Output> Stream)
{
	uint vertexId0 = input[0].vertexId;
	uint vertexId1 = input[1].vertexId;
	uint vertexId2 = input[2].vertexId;
	
	Vertex vertex0 = vertexBuffer[vertexId0];
    Vertex vertex1 = vertexBuffer[vertexId1];
    Vertex vertex2 = vertexBuffer[vertexId2];
    
	float3 rawTangent0 = vertex1.position - vertex0.position;
	float3 rawTangent1 = vertex2.position - vertex0.position;
	
    float3 normal = cross(rawTangent0, rawTangent1);
    if (dot(normal, normal) < EPSILON)
        normal = float3(0.0f, 0.0f, 1.0f);
    normal = normalize(normal);
	
	float3 testTangent0 = cross(normal, float3(0.0f, 0.0f, 1.0f));
	float3 testTangent1 = cross(normal, float3(0.0f, 1.0f, 0.0f));
	
    float3 tangent = (length(testTangent0) > length(testTangent1)) ? testTangent0 : testTangent1;
	tangent = normalize(tangent);
    
    float3 binormal = cross(tangent, normal);
	binormal = normalize(binormal);
    
	vertex0.normal = normal;
	vertex0.tangent = tangent;
	vertex0.binormal = binormal;
	vertex1.normal = normal;
	vertex1.tangent = tangent;
	vertex1.binormal = binormal;
	vertex2.normal = normal;
	vertex2.tangent = tangent;
	vertex2.binormal = binormal;
	
	vertexBuffer[vertexId0] = vertex0;
	vertexBuffer[vertexId1] = vertex1;
	vertexBuffer[vertexId2] = vertex2;
}
