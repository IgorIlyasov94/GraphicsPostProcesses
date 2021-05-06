#pragma once

#include "GraphicsHelper.h"

namespace Graphics
{
	class Camera
	{
	public:
		Camera(float fovAngleY, float aspectRatio, float zNear, float zFar);
		~Camera();

		void Move(float3 _position);
		void MoveRelative(float3 velocity);

		void LookAt(float3 target);

		bool BoundingBoxInScope(const BoundingBox& boundingBox) const;

		const float4x4& GetViewProjection();

		void Update();

	private:
		Camera() = delete;

		void UpdateMatrices();

		using Frustum = std::array<floatN, 6>;

		void UpdateFrustum(const float4x4& _viewProjection, Frustum& _frustum);

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

		Frustum frustum;
	};
}
