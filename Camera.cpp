#include "Camera.h"

Graphics::Camera::Camera(float fovAngleY, float aspectRatio, float _zNear, float _zFar)
	: viewProjection{}, view{}, projection{}, invViewProjection{}, invView{}, invProjection{},
	position{ 0.0f, 0.0f, 0.0f }, lookAtPoint{ 0.0f, 0.0f, 1.0f }, upVector{ 0.0f, 1.0f, 0.0f },
	zNear(_zNear), zFar(_zFar), fovY(fovAngleY), frustum{}
{
	projection = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, zNear, zFar);
	invProjection = XMMatrixInverse(nullptr, projection);

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

const float4x4& Graphics::Camera::GetView() const
{
	return view;
}

const float4x4& Graphics::Camera::GetProjection() const
{
	return projection;
}

const float4x4& Graphics::Camera::GetViewProjection() const
{
	return viewProjection;
}

const float4x4& Graphics::Camera::GetInvView() const
{
	return invView;
}

const float4x4& Graphics::Camera::GetInvProjection() const
{
	return invProjection;
}

const float4x4& Graphics::Camera::GetInvViewProjection() const
{
	return invViewProjection;
}

const float3& Graphics::Camera::GetPosition() const
{
	return position;
}

const Graphics::Camera::Frustum& Graphics::Camera::GetFrustum() const
{
	return frustum;
}

const std::array<floatN, 8>& Graphics::Camera::GetFrustumVertices() const
{
	return frustumVertices;
}

const float& Graphics::Camera::GetZNear() const
{
	return zNear;
}

const float& Graphics::Camera::GetZFar() const
{
	return zFar;
}

const float& Graphics::Camera::GetFovY() const
{
	return fovY;
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
	invView = XMMatrixInverse(nullptr, view);
	invViewProjection = XMMatrixInverse(nullptr, viewProjection);
}

void Graphics::Camera::UpdateFrustum(const float4x4& _viewProjection, Frustum& _frustum)
{
	for (size_t halfFrustumId = 0; halfFrustumId < _frustum.size() / 2; halfFrustumId++)
	{
		floatN frustumPlane = _viewProjection.r[3] + _viewProjection.r[halfFrustumId];
		_frustum[halfFrustumId * 2] = XMPlaneNormalizeEst(frustumPlane);

		frustumPlane = _viewProjection.r[3] - _viewProjection.r[halfFrustumId];
		_frustum[halfFrustumId * 2 + 1] = XMPlaneNormalizeEst(frustumPlane);
	}

	FrustumVertices(frustum, frustumVertices);
}

float UnlinearizeZ(float z)
{
	float zNear = 0.01f;
	float zFar = 1024.0f;

	z = zNear + (zFar - zNear) * z;

	float Pz = (1.0f - zFar / zNear) / zFar;
	float Pw = 1.0f / zNear;

	return (1.0f / z - Pw) / Pz;
}

void Graphics::Camera::FrustumVertices(const Frustum& _frustum, std::array<floatN, 8>& _frustumVertices)
{
	_frustumVertices[0] = { -1.0f, 1.0f, 0.0f, 1.0f };
	_frustumVertices[1] = { 1.0f, 1.0f, 0.0, 1.0f };
	_frustumVertices[2] = { -1.0f, -1.0f, 0.0f, 1.0f };
	_frustumVertices[3] = { 1.0f, -1.0f, 0.0f, 1.0f };
	_frustumVertices[4] = { -1.0f, 1.0f, 1.0f, 1.0f };
	_frustumVertices[5] = { 1.0f, 1.0f, 1.0f, 1.0f };
	_frustumVertices[6] = { -1.0f, -1.0f, 1.0f, 1.0f };
	_frustumVertices[7] = { 1.0f, -1.0f, 1.0f, 1.0f };

	for (auto& frustumVertice : _frustumVertices)
	{
		frustumVertice = XMVector4Transform(frustumVertice, invProjection);

		frustumVertice.m128_f32[0] /= frustumVertice.m128_f32[3];
		frustumVertice.m128_f32[1] /= frustumVertice.m128_f32[3];
		frustumVertice.m128_f32[2] /= frustumVertice.m128_f32[3];
		frustumVertice.m128_f32[3] /= frustumVertice.m128_f32[3];
	}
}

bool Graphics::Camera::IntersectsBoundingBoxAndPlane(const std::array<floatN, 8>& boundingBoxVertices, const floatN& plane) const
{
	for (auto& point : boundingBoxVertices)
		if (XMPlaneDotCoord(plane, point).m128_f32[0] >= 0.0f)
			return true;

	return false;
}

