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

Output main(Input input, uint vertexId : SV_VertexID)
{
	Output output = (Output)0;
	
	output.position = float4(input.position.x, input.position.y, 0.5f, 1.0f);
	output.texCoord = input.texCoord;
	
	return output;
}