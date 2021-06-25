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
	float padding;
};

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
	float intensity;
};

StructuredBuffer<BoundingBox> clusterData : register(t0);
StructuredBuffer<PointLight> pointLightBuffer : register(t1);
RWTexture3D<uint> pointLightCluster : register(u0);

static const uint CLUSTER_SIZE_X = 8;
static const uint CLUSTER_SIZE_Y = 6;
static const uint CLUSTER_SIZE_Z = 6;
static const uint CLUSTER_SIZE = CLUSTER_SIZE_X * CLUSTER_SIZE_Y * CLUSTER_SIZE_Z;
static const uint3 CLUSTER_SIZE_XYZ = uint3(CLUSTER_SIZE_X, CLUSTER_SIZE_Y, CLUSTER_SIZE_Z);
static const uint CLUSTER_LIGHTS_PER_CELL = 128;

bool IntersectionSphereAndBoundingBox(Sphere sphere, BoundingBox boundingBox)
{
	float sqDistance = 0.0f;

    for ( int i = 0; i < 3; ++i )
    {
        float v = sphere.center[i];

        if ( v < boundingBox.minCornerPoint[i] ) sqDistance += pow( boundingBox.minCornerPoint[i] - v, 2 );
        if ( v > boundingBox.maxCornerPoint[i] ) sqDistance += pow( v - boundingBox.maxCornerPoint[i], 2 );
    }
	
    return sqDistance <= sphere.radius * sphere.radius;
	
	/*float3 closestPoint = max(boundingBox.minCornerPoint, min(boundingBox.maxCornerPoint, sphere.center));
	float closestDistance = length(sphere.center - closestPoint);
	
	return closestDistance <= sphere.radius;*/
}

uint3 UnpackAddress(uint flatAddress, uint3 size)
{
	uint sizeXY = size.x * size.y;
	
	return uint3(flatAddress % size.x, (flatAddress % sizeXY) / size.x, flatAddress / sizeXY);
}

groupshared PointLight currentPointLight;
groupshared BoundingBox tempClusterData[CLUSTER_SIZE];

[numthreads(CLUSTER_SIZE, 1, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID, uint3 groupThreadId : SV_GroupThreadId, uint3 groupId : SV_GroupId, uint groupIndex : SV_GroupIndex)
{
	if (groupIndex == 0)
	{
		currentPointLight = pointLightBuffer[groupId.x];
		
		float4 viewSpacePointLightCenter = mul(view, float4(currentPointLight.position, 1.0f));
		currentPointLight.position = viewSpacePointLightCenter.xyz / viewSpacePointLightCenter.w;
	}
	
	tempClusterData[groupIndex] = clusterData[groupIndex];
	
	GroupMemoryBarrierWithGroupSync();
	
	Sphere pointLightSphere;
	pointLightSphere.center = currentPointLight.position;
	pointLightSphere.radius = currentPointLight.radius;
	
	if (IntersectionSphereAndBoundingBox(pointLightSphere, tempClusterData[groupIndex]))
	{
		uint3 resultAddress = UnpackAddress(groupIndex, uint3(CLUSTER_SIZE_X, CLUSTER_SIZE_Y, CLUSTER_SIZE_Z));
		resultAddress = clamp(resultAddress, uint3(0, 0, 0), CLUSTER_SIZE_XYZ - uint3(1, 1, 1));
		resultAddress.x *= CLUSTER_LIGHTS_PER_CELL;
		
		uint originalValue;
		InterlockedAdd(pointLightCluster[resultAddress], 1u, originalValue);
		resultAddress.x += originalValue + 1;
		
		pointLightCluster[resultAddress] = groupId.x;
	}
}
