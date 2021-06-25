Texture2D texScene : register(t0);
Texture2D texBloom : register(t1);

SamplerState sLinear : register(s0);

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

Output main(Input input)
{
	Output output = (Output)0;
	
	float3 color = texScene.Load(int3((int2) input.position.xy, 0)).xyz;
	
	float gray = dot(color, float3(0.2125f, 0.7154f, 0.0721f));
	
	float luminance = 0.3f;
	
	float3 bloom = texBloom.SampleLevel(sLinear, input.texCoord, 0.0f).xyz;
	
	float shiftCoefficient = 1.0f - (luminance + 1.5f) / 4.1f;
	shiftCoefficient = saturate(shiftCoefficient);
	
	float3 rodColor = gray * shiftVector;
	color = lerp(color, rodColor, shiftCoefficient);
	
	color = (color * middleGray) / (luminance + 0.001f);
	color *= 1.0f + color / (whiteCutoff * whiteCutoff);
	color /= 1.0f + color;
	/*
	float4 outputColor = 0.0f.xxxx;
	
	if (WaveIsFirstLane())
		outputColor = float4(1., 1., 1., 1.);
	if (WaveGetLaneIndex() == WaveActiveMax(WaveGetLaneIndex()))
		outputColor = float4(1., 0., 0., 1.);
	
	outputColor.xyz = color;//saturate(color + bloom * shiftVector);// + shiftVector;
	*/
	output.color = float4(saturate(color + bloom), 1.0f);
	
	return output;
}
