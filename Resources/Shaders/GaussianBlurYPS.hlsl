Texture2D texScene : register(t0);

struct Input
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD0;
};

struct Output
{
	float4 color : SV_TARGET;
};

static const uint KERNEL_SIZE = 17;
static const float PI = 3.14159265359f;

float CalculateWeight(uint kernelSize, int texelId, float sigma)
{
	float weight;
	
	float kernelRadius = (float)(kernelSize / 2);
	
	float xShift = (float)texelId - kernelRadius;
		
	weight = exp(-xShift * xShift / (2.0f * sigma * sigma)) / (sigma * sqrt(2.0f * PI));
	
	return weight;
}

Output main(Input input)
{
	Output output = (Output)0;
	
	float3 color = 0.0f.xxx;
	
	[unroll]
	for (int texelId = 0; texelId < KERNEL_SIZE; texelId++)
	{
		int shift = texelId - KERNEL_SIZE / 2;
		float weight = CalculateWeight(KERNEL_SIZE, texelId, 3.0f);
		
		color += texScene.Load(int3((int2) input.position.xy + int2(0, shift), 0)).xyz * weight;
	}
	
	output.color = float4(saturate(color), 1.0f);
	
	return output;
}
