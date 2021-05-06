#include "Camera.h"

Graphics::Camera::Camera(float fovAngleY, float aspectRatio, float zNear, float zFar)
	: viewProjection{}, view{}, projection{}, invViewProjection{}, invView{}, invProjection{},
	position{ 0.0f, 0.0f, 0.0f }, lookAtPoint{ 0.0f, 0.0f, 1.0f }, upVector{ 0.0f, 1.0f, 0.0f },
	frustum{}
{
	projection = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, zNear, zFar);
	invProjection = XMMatrixInverse(nullptr, XMMatrixTranspose(projection));

	UpdateMatrices();
	UpdateFrustum(viewProjection, frustum);
}

Graphics::Camera::~Camera()
{

}

void Graphics::Camera::Move(float3 _position)
{
	position = _position;
}

void Graphics::Camera::MoveRelative(float3 velocity)
{
	position.x += velocity.x;
	position.y += velocity.y;
	position.z += velocity.z;
}

void Graphics::Camera::LookAt(float3 target)
{
	lookAtPoint = target;
}

bool Graphics::Camera::BoundingBoxInScope(const BoundingBox& boundingBox) const
{
	std::array<floatN, 8> boundingBoxVertices;
	BoundingBoxVertices(boundingBox, boundingBoxVertices);

	for (auto& frustumPlane : frustum)
	{
		if (IntersectsBoundingBoxAndPlane(boundingBoxVertices, frustumPlane))
			continue;

		return false;
	}

	return true;
}

const float4x4& Graphics::Camera::GetViewProjection()
{
	return viewProjection;
}

void Graphics::Camera::Update()
{
	UpdateMatrices();
	UpdateFrustum(viewProjection, frustum);
}

void Graphics::Camera::UpdateMatrices()
{
	view = XMMatrixLookAtLH(XMLoadFloat3(&position), XMLoadFloat3(&lookAtPoint), XMLoadFloat3(&upVector));
	viewProjection = XMMatrixMultiply(view, projection);
	invView = XMMatrixInverse(nullptr, XMMatrixTranspose(view));
	invViewProjection = XMMatrixInverse(nullptr, viewProjection);
}

void Graphics::Camera::UpdateFrustum(const float4x4& _viewProjection, Frustum& _frustum)
{
	for (uint32_t halfFrustumId = 0; halfFrustumId < _frustum.size() / 2; halfFrustumId++)
	{
		floatN frustumPlane = _viewProjection.r[3] + _viewProjection.r[halfFrustumId];
		_frustum[halfFrustumId * 2] = XMPlaneNormalizeEst(frustumPlane);

		frustumPlane = _viewProjection.r[3] - _viewProjection.r[halfFrustumId];
		_frustum[halfFrustumId * 2 + 1] = XMPlaneNormalizeEst(frustumPlane);
	}
}

bool Graphics::Camera::IntersectsBoundingBoxAndPlane(const std::array<floatN, 8>& boundingBoxVertices, const floatN& plane) const
{
	for (auto& point : boundingBoxVertices)
		if (XMPlaneDotCoord(plane, point).m128_f32[0] >= 0.0f)
			return true;

	return false;
}

