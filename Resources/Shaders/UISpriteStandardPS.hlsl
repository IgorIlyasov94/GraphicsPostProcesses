cbuffer LocalConstBuffer : register(b0)
{
	float2 screenCoordOffset;
	float2 scale;
	float4 color;
};

Texture2D<float4> spriteTexture : register(t0);

SamplerState sLinear : register(s0);

struct Input
{
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD0;
};

struct Output
{
	float4 color : SV_Target0;
};

Output main(Input input)
{
	Output output = (Output)0;
	
	float4 resultColor = spriteTexture.Sample(sLinear, input.texCoord);
	resultColor *= color;
	
	output.color = resultColor;
	
	return output;
}
