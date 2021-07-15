cbuffer LocalConstBuffer : register(b0)
{
	float2 screenCoordOffset;
	float2 scale;
	float4 color;
};

struct Input
{
	float2 localScreenCoordOffset : POSITION;
	float2 localScale : TEXCOORD0;
	float2 texCoordOffset : TEXCOORD1;
	float2 texCoordScale : TEXCOORD2;
	uint vertexId : SV_VertexId;
};

struct Output
{
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD0;
};

Output main(Input input)
{
	Output output = (Output)0;
	
	float2 texCoord = float2(input.vertexId & 1, (input.vertexId >> 1) & 1);
	float2 position = float2(texCoord.x, -texCoord.y);
	
	output.position = float4(position * input.localScale * scale + input.localScreenCoordOffset + screenCoordOffset, 0.0f, 1.0f);
	output.texCoord = texCoord * input.texCoordScale + input.texCoordOffset;
	
	return output;
}
