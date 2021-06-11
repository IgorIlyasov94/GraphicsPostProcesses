struct Sphere
{
	float3 center;
	float radius;
};

struct BoundingBox
{
	float4 minCornerPoint;
	float4 maxCornerPoint;
};

struct PointLight
{
	float3 position;
	float radius;
	float3 color;
	uint intensity;
};

StructuredBuffer<BoundingBox> clusterData : register(t0);
StructuredBuffer<PointLight> pointLightBuffer : register(t1);
RWTexture3D<uint> pointLightCluster : register(u0);

static const uint CLUSTER_SIZE_X = 8;
static const uint CLUSTER_SIZE_Y = 6;
static const uint CLUSTER_SIZE_Z = 6;
static const uint CLUSTER_SIZE = CLUSTER_SIZE_X * CLUSTER_SIZE_Y * CLUSTER_SIZE_Z;
static const uint CLUSTER_LIGHTS_PER_CELL = 256;

bool IntersectionSphereAndBoundingBox(Sphere sphere, BoundingBox boundingBox)
{
	float3 closestPoint;
	closestPoint.x = max(boundingBox.minCornerPoint.x, min(boundingBox.maxCornerPoint.x, sphere.center.x));
	closestPoint.y = max(boundingBox.minCornerPoint.y, min(boundingBox.maxCornerPoint.y, sphere.center.y));
	closestPoint.z = max(boundingBox.minCornerPoint.z, min(boundingBox.maxCornerPoint.z, sphere.center.z));
	
	float dist = length(sphere.center - closestPoint);
	
	return dist < sphere.radius;
}

bool ClusterAddressFromPosition(StructuredBuffer<BoundingBox> clusterCoords, Sphere pointLightSphere, out uint flatAddress)
{
	flatAddress = 0;
	
	bool success = false;
	
	for (uint clusterCellId = 0; clusterCellId < CLUSTER_SIZE; clusterCellId++)
	{
		if (IntersectionSphereAndBoundingBox(pointLightSphere, clusterCoords[clusterCellId]))
		{
			flatAddress = clusterCellId;
			
			success = true;
		}
	}
	
	return success;
}

uint3 UnpackAddress(uint flatAddress, uint3 size)
{
	uint sizeXY = size.x * size.y;
	
	return uint3(flatAddress % size.x, (flatAddress % sizeXY) / size.y, flatAddress / sizeXY);
}

[numthreads(16, 1, 1)]
//[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
	PointLight pointLight = pointLightBuffer[dispatchThreadId.x];
	
	uint flatAddress = 0;
	Sphere pointLightSphere;
	pointLightSphere.center = pointLight.position;
	pointLightSphere.radius = pointLight.radius;
	
	if (!ClusterAddressFromPosition(clusterData, pointLightSphere, flatAddress))
		return;
	
	/*if (!IntersectionSphereAndBoundingBox(pointLightSphere, clusterData[dispatchThreadId.y]))
		return;*/
	
	uint3 resultAddress = UnpackAddress(flatAddress, uint3(CLUSTER_SIZE_X, CLUSTER_SIZE_Y, CLUSTER_SIZE_Z));
	//uint3 resultAddress = UnpackAddress(dispatchThreadId.y, uint3(CLUSTER_SIZE_X, CLUSTER_SIZE_Y, CLUSTER_SIZE_Z));
	resultAddress.x *= CLUSTER_LIGHTS_PER_CELL;
	
	uint pointLightNumber = pointLightCluster[resultAddress];
	
	uint originalValue;
	InterlockedAdd(pointLightCluster[resultAddress], 1u, originalValue);
	
	resultAddress.x += pointLightNumber;
	
	pointLightCluster[resultAddress] = dispatchThreadId.x;
}
