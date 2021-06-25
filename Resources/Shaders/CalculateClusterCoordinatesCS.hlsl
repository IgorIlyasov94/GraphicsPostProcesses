cbuffer CB : register(b0)
{
	float4x4 invView;
	float4x4 invProj;
	float2 zLinearizeCoeff;
	float zNear;
	float zFar;
	float3 camPosition;
	float fovY;
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

bool IntersectionLineAndPlane(float3 startLinePoint, float3 endLinePoint, float4 plane, out float3 intersectionPoint)
{
	intersectionPoint = float3(0.0f, 0.0f, 0.0f);
	
	float3 lineDirection = endLinePoint - startLinePoint;
	
	float intersectionCoeff = (plane.w - dot(plane.xyz, startLinePoint)) / dot(plane.xyz, lineDirection);
	
	bool hasIntersection = intersectionCoeff >= 0.0f && intersectionCoeff <= 1.0f;
	
	if (hasIntersection)
		intersectionPoint = startLinePoint + intersectionCoeff * lineDirection;
		
	return hasIntersection;
}

[numthreads(16, 1, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
	uint3 address3D = UnpackAddress(dispatchThreadId.x, CLUSTER_SIZE_XYZ);
	//address3D = clamp(address3D, uint3(0, 0, 0), CLUSTER_SIZE_XYZ - uint3(1, 1, 1));
	
	float4 clipCoordNear = CalculateClipSpacePoint(address3D, CLUSTER_SIZE_XYZ, zLinearizeCoeff, zNear, zFar);
	float4 clipCoordFar = CalculateClipSpacePoint(address3D + 1, CLUSTER_SIZE_XYZ, zLinearizeCoeff, zNear, zFar);
	
	float4 viewCoordNear = ConvertClipToView(clipCoordNear, invProj);
	float4 viewCoordFar = ConvertClipToView(clipCoordFar, invProj);
	
	float clusterZNear = 1.0f + 2.0f * tan(fovY / 2.0f) / (float) CLUSTER_SIZE_XYZ.y;
	
	float4 clusterNearPlane = float4(0.0f, 0.0f, 1.0f, clipCoordNear.z);//+zNear * pow(abs(clusterZNear), (float) address3D.z));
	float4 clusterFarPlane = float4(0.0f, 0.0f, 1.0f, clipCoordFar.z);//-zNear * pow(abs(clusterZNear), (float) (address3D.z + 1)));
	
	float3 cameraOrigin = float3(0.0f, 0.0f, 0.0f);
	
	float3 minNearPoint;
	float3 maxNearPoint;
	float3 minFarPoint;
	float3 maxFarPoint;
	
	bool a = IntersectionLineAndPlane(cameraOrigin, viewCoordNear.xyz, clusterNearPlane, minNearPoint);
	bool b = IntersectionLineAndPlane(cameraOrigin, viewCoordFar.xyz, clusterNearPlane, maxNearPoint);
	bool c = IntersectionLineAndPlane(cameraOrigin, viewCoordNear.xyz, clusterFarPlane, minFarPoint);
	bool d = IntersectionLineAndPlane(cameraOrigin, viewCoordFar.xyz, clusterFarPlane, maxFarPoint);
	
	minNearPoint = ConvertClipToView(clipCoordNear, invProj);
	maxNearPoint = ConvertClipToView(float4(clipCoordFar.xy, clipCoordNear.z, 1.0f), invProj);
	minFarPoint = ConvertClipToView(float4(clipCoordNear.xy, clipCoordFar.z, 1.0f), invProj);
	maxFarPoint = ConvertClipToView(clipCoordFar, invProj);
	
	viewCoordNear.xyz = min(minNearPoint, min(maxNearPoint, min(minFarPoint, maxFarPoint)));
	viewCoordNear.w = 1.0f;
	
	viewCoordFar.xyz = max(minNearPoint, max(maxNearPoint, max(minFarPoint, maxFarPoint)));
	viewCoordFar.w = 1.0f;
	
	float4 worldCoordNear = mul(invView, viewCoordNear);
	float4 worldCoordFar = mul(invView, viewCoordFar);
	
	/*worldCoordNear /= worldCoordNear.w;
	worldCoordFar /= worldCoordFar.w;
	
	worldCoordNear.xyz = float3(min(worldCoordNear.x, worldCoordFar.x), min(worldCoordNear.y, worldCoordFar.y), min(worldCoordNear.z, worldCoordFar.z));
	worldCoordFar.xyz = float3(max(worldCoordNear.x, worldCoordFar.x), max(worldCoordNear.y, worldCoordFar.y), max(worldCoordNear.z, worldCoordFar.z));*/
	
	BoundingBox resultClusterCellData;
	resultClusterCellData.minCornerPoint = viewCoordNear;//float4(a, b, c, 1.0f);//
	resultClusterCellData.maxCornerPoint = viewCoordFar;//float4(d, d, d, 1.0f);//
	
	clusterData[dispatchThreadId.x] = resultClusterCellData;
}
