cbuffer CB : register(b0)
{
	float4x4 invViewProj;
};

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
	
	return uint3(flatAddress % size.x, (flatAddress % sizeXY) / size.y, flatAddress / sizeXY);
}

float3 CalculateScreenSpacePoint(uint3 address3D, uint3 clusterSize)
{
	float3 resultPoint = address3D / (float3) clusterSize;
	resultPoint.xy = resultPoint.xy * 2.0f - 1.0f.xx;
	
	return resultPoint;
}

float4 ConvertScreenToWorld(float3 screenCoord, float4x4 invViewProjection)
{
	float4 worldCoord = mul(invViewProjection, screenCoord);
	
	return worldCoord;
}

[numthreads(16, 1, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
	uint3 address3D = UnpackAddress(dispatchThreadId.x, CLUSTER_SIZE_XYZ);
	
	BoundingBox resultClusterCellData;
	resultClusterCellData.minCornerPoint = float4(CalculateScreenSpacePoint(address3D, CLUSTER_SIZE_XYZ), 1.0f);
	resultClusterCellData.maxCornerPoint = float4(CalculateScreenSpacePoint(address3D + 1, CLUSTER_SIZE_XYZ), 1.0f);
	
	resultClusterCellData.minCornerPoint = ConvertScreenToWorld(resultClusterCellData.minCornerPoint, invViewProj);
	resultClusterCellData.maxCornerPoint = ConvertScreenToWorld(resultClusterCellData.maxCornerPoint, invViewProj);
	
	clusterData[dispatchThreadId.x] = resultClusterCellData;
}
