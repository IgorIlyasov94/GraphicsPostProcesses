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
	float4 randomValues;
	float3 cameraPosition;
	float elapsedTime;
	float previousElapsedTime;
	float3 padding;
};
