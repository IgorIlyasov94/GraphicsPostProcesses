struct Input
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD;
};

struct Output
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD0;
};

static const float2 vertices[4] =
{
	float2(-1.0f, 1.0f),
	float2(1.0f, 1.0f),
	float2(-1.0f, -1.0f),
	float2(1.0f, -1.0f)
};

static const float2 texCoords[4] =
{
	float2(0.0f, 0.0f),
	float2(1.0f, 0.0f),
	float2(0.0f, 1.0f),
	float2(1.0f, 1.0f)
};

Output main(Input input, uint vertexId : SV_VertexID)
{
	Output output = (Output)0;
	
	output.position = float4(input.position.x, input.position.y, 0.5f, 1.0f);
	output.texCoord = input.texCoord;
	
	/*output.position.xy = vertices[vertexId];
	output.position.zw = float2(0.5f, 1.0);
	
	output.texCoord = texCoords[vertexId];
	*/
	return output;
}