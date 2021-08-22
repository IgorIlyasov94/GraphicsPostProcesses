#include "GlobalConstants.hlsli"

struct BoundingBox
{
	float4 minCornerPoint;
	float4 maxCornerPoint;
};

RWStructuredBuffer<BoundingBox> clusterData : register(u0);

static const uint CLUSTER_SIZE_X = 8;
static const uint CLUSTER_SIZE_Y = 6;
static const uint CLUSTER_SIZE_Z = 6;
static const uint CLUSTER_SIZE = CLUSTER_SIZE_X * CLUSTER_SIZE_Y * CLUSTER_SIZE_Z;
static const uint3 CLUSTER_SIZE_XYZ = uint3(CLUSTER_SIZE_X, CLUSTER_SIZE_Y, CLUSTER_SIZE_Z);

uint3 UnpackAddress(uint flatAddress, uint3 size)
{
	uint sizeXY = size.x * size.y;
	
	return uint3(flatAddress % size.x, (flatAddress % sizeXY) / size.x, flatAddress / sizeXY);
}

float UnlinearizeZ(float linearZ, float2 linearizeCoeff)
{
	return (1.0f / linearZ - linearizeCoeff.y) / linearizeCoeff.x;
}

float4 CalculateClipSpacePoint(uint3 address3D, uint3 clusterSize, float2 linearizeCoeff, float _zNear, float _zFar)
{
	float4 resultPoint;
	resultPoint.xyz = saturate(address3D / (float3) clusterSize);
	resultPoint.y = 1.0f - resultPoint.y;
	resultPoint.xy = resultPoint.xy * 2.0f - 1.0f.xx;
	resultPoint.z = lerp(_zNear, _zFar, resultPoint.z);
	resultPoint.z = UnlinearizeZ(resultPoint.z, linearizeCoeff);
	resultPoint.w = 1.0f;
	
	return resultPoint;
}

float4 ConvertClipToView(float4 clipCoord, float4x4 _invProj)
{
	float4 viewCoord = mul(_invProj, clipCoord);
	viewCoord /= viewCoord.w;
	
	return viewCoord;
}

[numthreads(16, 1, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
	uint3 address3D = UnpackAddress(dispatchThreadId.x, CLUSTER_SIZE_XYZ);
	address3D = clamp(address3D, uint3(0, 0, 0), CLUSTER_SIZE_XYZ - uint3(1, 1, 1));
	
	float4 clipCoordNear = CalculateClipSpacePoint(address3D, CLUSTER_SIZE_XYZ, zLinearizeCoeff, zNear, zFar);
	float4 clipCoordFar = CalculateClipSpacePoint(address3D + 1, CLUSTER_SIZE_XYZ, zLinearizeCoeff, zNear, zFar);
	
	float3 minNearPoint = ConvertClipToView(clipCoordNear, invProjection);
	float3 maxNearPoint = ConvertClipToView(float4(clipCoordFar.xy, clipCoordNear.z, 1.0f), invProjection);
	float3 minFarPoint = ConvertClipToView(float4(clipCoordNear.xy, clipCoordFar.z, 1.0f), invProjection);
	float3 maxFarPoint = ConvertClipToView(clipCoordFar, invProjection);
	
	float4 viewCoordNear = float4(min(minNearPoint, min(maxNearPoint, min(minFarPoint, maxFarPoint))), 1.0f);
	float4 viewCoordFar = float4(max(minNearPoint, max(maxNearPoint, max(minFarPoint, maxFarPoint))), 1.0f);
	
	BoundingBox resultClusterCellData;
	resultClusterCellData.minCornerPoint = viewCoordNear;
	resultClusterCellData.maxCornerPoint = viewCoordFar;
	
	clusterData[dispatchThreadId.x] = resultClusterCellData;
}
