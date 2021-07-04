cbuffer ImmutableGlobalConstBuffer : register(b0)
{
	float4x4 projection;
	float4x4 invProjection;
	float zNear;
	float zFar;
	float2 zLinearizeCoeff;
};

cbuffer GlobalConstBuffer : register(b1)
{
	float4x4 view;
	float4x4 invView;
	float4x4 viewProjection;
	float4x4 invViewProjection;
	float3 cameraPosition;
	float elapsedTime;
	float4 randomValues;
};

cbuffer LocalConstBuffer : register(b2)
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
	float4 clipCoord : TEXCOORD1;
	float4 worldCoord : TEXCOORD2;
};

Output main(Input input)
{
	Output output = (Output)0;
	
	output.position = mul(worldViewProj, float4(input.position, 1.0f));
	output.normal = normalize(mul((float3x3) world, input.normal).xyz);
	output.texCoord = input.texCoord;
	output.clipCoord = output.position;
	output.worldCoord = mul(world, float4(input.position, 1.0f));
	
	return output;
}
