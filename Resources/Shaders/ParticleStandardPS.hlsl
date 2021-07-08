Texture2D<float4> textureAtlas : register(t1);

SamplerState sLinear : register(s0);

struct Input
{
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD0;
	float4 color : COLOR;
};

struct Output
{
	float4 color : SV_Target0;
	float4 normal : SV_Target1;
};

//[earlydepthstencil]
Output main(Input input)
{
	Output output = (Output)0;
	
	float4 color = textureAtlas.Sample(sLinear, input.texCoord);
	color *= input.color;
	
	output.color = color;
	output.normal = float4(0.0f, 0.0f, -1.0f, 1.0f);
	
	return output;
}
