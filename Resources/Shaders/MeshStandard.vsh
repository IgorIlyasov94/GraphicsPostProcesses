cbuffer CB : register(b0)
{
	float4x4 world;
	float4x4 worldViewProj;
};

struct Input
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
};

struct Output
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD0;
};

Output main(Input input, uint vertexId : SV_VertexID)
{
	Output output = (Output)0;
	
	output.position = mul(worldViewProj, float4(input.position, 1.0f));
	output.normal = normalize(mul((float3x3) world, input.normal).xyz);
	output.texCoord = input.texCoord;
	
	return output;
}
