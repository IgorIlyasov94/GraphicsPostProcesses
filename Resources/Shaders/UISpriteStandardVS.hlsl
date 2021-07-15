cbuffer LocalConstBuffer : register(b0)
{
	float2 screenCoordOffset;
	float2 scale;
	float4 color;
};

struct Input
{
	float3 position : POSITION;
};

struct Output
{
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD0;
};

Output main(Input input)
{
	Output output = (Output)0;
	
	output.position = float4(input.position.xy * scale + screenCoordOffset, 0.0f, 1.0f);
	output.texCoord = input.position.xy;
	
	return output;
}
