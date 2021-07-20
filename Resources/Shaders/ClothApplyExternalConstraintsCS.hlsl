struct Constraint
{
	float4 constraintData0;
	float4 constraintData1;
	float3 constraintData2;
	uint constraintType;
};

struct Vertex
{
	float3 position;
	float3 previousPosition;
	float3 normal;
	float3 tangent;
	float3 binormal;
	float2 texCoord;
	uint isFree;
};

StructuredBuffer<Constraint> constraintBuffer : register(t0);
RWStructuredBuffer<Vertex> vertexBuffer : register(u0);

static const uint CONSTRAINT_TYPE_PLANE = 0;
static const uint CONSTRAINT_TYPE_SPHERE = 1;
static const uint CONSTRAINT_TYPE_CAPSULE = 2;

float3 AvoidPlane(float3 position, float3 planeNormal, float planeDistance)
{
	float3 displace = 0.0f;
	float distance = dot(position, planeNormal) + planeDistance;
	
    if (distance < 0.0f)
        displace = -distance * planeNormal;
	
	return displace;
}

float3 AvoidSphere(float3 position, float3 sphereCenter, float sphereRadius)
{
	float3 displace = 0.0f;
	float3 delta = position - sphereCenter;
    float distance = length(delta);
	
    if (distance < sphereRadius)
        displace = (sphereRadius - distance) * delta / distance;
	
	return displace;
}

float3 AvoidCapsule(float3 position, float3 origin, float3 axis, float coreLength, float2 radius)
{
	float3 displace = 0.0f;
	float3 delta = position - origin;
    float nearestCorePointCoeff = clamp(dot(delta, axis), 0.0f, coreLength);
    float3 corePoint = origin + nearestCorePointCoeff * axis;
	
    displace = AvoidSphere(position, corePoint, lerp(radius.x, radius.y, nearestCorePointCoeff / coreLength));
	
	return displace;
}

float3 AvoidConstraint(float3 position, Constraint constraint)
{
	float3 newPosition = position;
	
	if (constraint.constraintType == CONSTRAINT_TYPE_PLANE)
		newPosition += AvoidPlane(position, constraint.constraintData0.xyz, constraint.constraintData0.w);
	else if (constraint.constraintType == CONSTRAINT_TYPE_SPHERE)
		newPosition += AvoidSphere(position, constraint.constraintData0.xyz, constraint.constraintData0.w);
	else
		newPosition += AvoidCapsule(position, constraint.constraintData0.xyz, constraint.constraintData1.xyz, constraint.constraintData0.w, constraint.constraintData2.xy);
	
	return newPosition;
}

[numthreads(1, 1, 1)]
void main(uint3 groupId : SV_GroupId)
{
	Vertex vertex = vertexBuffer[groupId.x];
	
	if (vertex.isFree == 1)
	{
		float3 newPosition = vertex.position;
		Constraint constraint = constraintBuffer[groupId.y];
		
		newPosition += AvoidConstraint(newPosition, constraint);
		
		vertexBuffer[groupId.x].position = newPosition;
	}
}
