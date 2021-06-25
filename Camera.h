#pragma once

#include "GraphicsHelper.h"

namespace Graphics
{
	class Camera
	{
	public:
		Camera(float fovAngleY, float aspectRatio, float _zNear, float _zFar);
		~Camera();

		void Move(float3 _position);
		void MoveRelative(float3 velocity);

		void LookAt(float3 target);

		bool BoundingBoxInScope(const BoundingBox& boundingBox) const;

		const float4x4& GetView() const;
		const float4x4& GetProjection() const;
		const float4x4& GetViewProjection() const;
		const float4x4& GetInvView() const;
		const float4x4& GetInvProjection() const;
		const float4x4& GetInvViewProjection() const;

		const float3& GetPosition() const;

		using Frustum = std::array<floatN, 6>;

		const Frustum& GetFrustum() const;
		const std::array<floatN, 8>& GetFrustumVertices() const;

		const float& GetZNear() const;
		const float& GetZFar() const;
		const float& GetFovY() const;

		void Update();

	private:
		Camera() = delete;

		void UpdateMatrices();

		void UpdateFrustum(const float4x4& _viewProjection, Frustum& _frustum);
		void FrustumVertices(const Frustum& _frustum, std::array<floatN, 8>& _frustumVertices);

		bool IntersectsBoundingBoxAndPlane(const std::array<floatN, 8>& boundingBoxVertices, const floatN& plane) const;

		float4x4 viewProjection;
		float4x4 view;
		float4x4 projection;
		float4x4 invViewProjection;
		float4x4 invView;
		float4x4 invProjection;

		float3 position;
		float3 lookAtPoint;
		float3 upVector;

		float zNear;
		float zFar;
		float fovY;

		Frustum frustum;

		std::array<floatN, 8> frustumVertices;
	};
}
