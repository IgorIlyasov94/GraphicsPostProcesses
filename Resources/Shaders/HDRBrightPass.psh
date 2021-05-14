Texture2D texScene : register(t0);

cbuffer CB : register(b0)
{
	float3 shiftVector;
	float elapsedTime;
	float middleGray;
	float whiteCutoff;
	float brightPassThreshold;
	float brightPassOffset;
};

struct Input
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD0;
};

struct Output
{
	float4 color : SV_TARGET;
};

static const int SIZE_MULTIPLIER = 4;

Output main(Input input)
{
	Output output = (Output)0;
	
	float3 color = texScene.Load(int3((int2) input.position.xy, 0) * SIZE_MULTIPLIER).xyz;
	
	float luminance = 0.3f;
	
	color = (color * middleGray) / (luminance + 0.001f);
	color *= (1.0f + color / (whiteCutoff * whiteCutoff));
	color -= brightPassThreshold;
	color = max(color, 0.0f);
	color /= (brightPassOffset + color);
	
	output.color = float4(saturate(color), 1.0f);
	
	return output;
}
