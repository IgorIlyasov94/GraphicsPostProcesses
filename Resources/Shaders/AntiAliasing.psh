Texture2D texScene : register(t0);
Texture2D normScene : register(t1);
Texture2D depthScene : register(t2);

SamplerState sLinear : register(s0);

cbuffer CB : register(b0)
{
	float2 pixelSize;
	float2 padding;
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

static const float DEPTH_REJECT = 0.0000001f;
static const uint KERNEL_SIZE = 3;

float CalculateAxisOffset(float depth0, float depth1, float depth2, float depth3)
{
	float depthDistance0 = depth1 - depth0;
	float depthDistance1 = depth3 - depth2;
	float augmentedDepth0 = depth1 + depthDistance0;
	float augmentedDepth1 = depth2 - depthDistance1;
		
	return (augmentedDepth1 - augmentedDepth0) / (depthDistance0 - depthDistance1);
}

float CalculateAxisOffset(float depth0, float depth1, float depth2, float depth3, float currentDepth, float depthReject)
{
	float depthDistance0 = depth1 - depth0;
	float depthDistance1 = depth3 - depth2;
	/*
	float augmentedDepth0 = depthDistance0;
	float augmentedDepth1 = -depthDistance1;
	
	if (abs(depth1 - currentDepth) > depthReject || abs(depth0 - currentDepth) > depthReject)
		augmentedDepth0 = -depth2 + depthDistance0;
	else
		augmentedDepth0 += depth1;
	
	if (abs(depth2 - currentDepth) > depthReject || abs(depth3 - currentDepth) > depthReject)
		augmentedDepth1 = -depth1 - depthDistance1;
	else
		augmentedDepth1 += depth2;
	
	return (augmentedDepth1 - augmentedDepth0) / (depthDistance0 - depthDistance1);*/
	
	float augmentedDepth0_0 = depth1 + depthReject * currentDepth - depthDistance0;
	float augmentedDepth1_0 = depth1 - depthDistance0;
	float augmentedDepth0_1 = depth2 + depthReject * currentDepth + depthDistance1;
	float augmentedDepth1_1 = depth2 - depthDistance1;
	
	float offset0 = (augmentedDepth1_0 - augmentedDepth0_0) / (-depthDistance0 - depthDistance0);
	float offset1 = (augmentedDepth1_1 - augmentedDepth0_1) / (-depthDistance1 - depthDistance0);
	
	float offset = (abs(offset0) < abs(offset1)) ? offset0 : offset1;
	
	return offset;
}

float2 FindOffset(Texture2D depthBuffer, float currentDepth, int2 texCoord, float depthReject)
{
	float2 offset = 0.5f.xx;
	float2 maxFlag = 0.0f.xx;
	
	int kernelRadius = KERNEL_SIZE / 2;
	
	float depthXL = depthBuffer.Load(int3(texCoord + int2(-1, 0), 0)).x;
	float depthXR = depthBuffer.Load(int3(texCoord + int2(1, 0), 0)).x;
	float depthX2L = depthBuffer.Load(int3(texCoord + int2(-2, 0), 0)).x;
	float depthX2R = depthBuffer.Load(int3(texCoord + int2(2, 0), 0)).x;
	
	bool isXEdge = abs(depthXL + depthXR - 2.0f * currentDepth) > 0.0000001f * currentDepth;
	bool isSilhouette = false;
	
	if (isXEdge)
	{
		offset.x = CalculateAxisOffset(depthX2L, depthXL, depthXR, depthX2R);
		
		isSilhouette = abs(offset.x) > 1.0f;
		
		offset.x = (abs(offset.x) < 0.5f) ? offset.x : 0.5f;
	}
	
	float depthYD = depthBuffer.Load(int3(texCoord + int2(0, -1), 0)).x;
	float depthYU = depthBuffer.Load(int3(texCoord + int2(0, 1), 0)).x;
	float depthY2D = depthBuffer.Load(int3(texCoord + int2(0, -2), 0)).x;
	float depthY2U = depthBuffer.Load(int3(texCoord + int2(0, 2), 0)).x;
	
	bool isYEdge = abs(depthYD + depthYU - 2.0f * currentDepth) > 0.0000001f * currentDepth;
	
	if (isYEdge)
	{
		offset.y = CalculateAxisOffset(depthY2D, depthYD, depthYU, depthY2U);
		
		isSilhouette = abs(offset.y) > 1.0f;
		
		offset.y = (abs(offset.y) < 0.5f) ? offset.y : 0.5f;
	}
	
	if (isSilhouette)
	{
		if (isXEdge)
		{
			maxFlag.x = offset.x;
			offset.x = CalculateAxisOffset(depthX2L, depthXL, depthXR, depthX2R, currentDepth, depthReject);
			offset.x *= min(abs(currentDepth - depthXL), abs(currentDepth - depthXR));
			offset.x = ((abs(offset.x) < 0.5f) ? offset.x : 0.5f);
		}
		
		if (isYEdge)
		{
			maxFlag.y = offset.y;
			offset.y = CalculateAxisOffset(depthY2D, depthYD, depthYU, depthY2U, currentDepth, depthReject);
			offset.y *= min(abs(currentDepth - depthYD), abs(currentDepth - depthYU));
			offset.y = ((abs(offset.y) < 0.5f) ? offset.y : 0.5f);
		}
	}
	
	if (abs(offset.x) > abs(offset.y))
	{
		offset.x = 0.0f;
		offset.y = ((offset.y >= 0.0f) ? 0.5f : -0.5f) - offset.y;
	}
	else
	{
		offset.x = ((offset.x >= 0.0f) ? 0.5f : -0.5f) - offset.x;
		offset.y = 0.0f;
	}
	
	return offset;
}

Output main(Input input)
{
	Output output = (Output)0;
	
	float depth = depthScene.Load(int3((int2) input.position.xy, 0)).x;
	
	float2 offset = FindOffset(depthScene, depth, (int2) input.position.xy, DEPTH_REJECT);
	offset *= pixelSize;
	
	float3 color = texScene.Sample(sLinear, input.texCoord + offset).xyz;
	
	output.color = float4(color, 1.0f);
	
	return output;
}
