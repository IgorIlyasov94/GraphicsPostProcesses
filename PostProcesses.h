#pragma once

#include "GraphicsHelper.h"
#include "ResourceManager.h"
#include "GraphicObject.h"

namespace Graphics
{
	class PostProcesses
	{
	public:
		static PostProcesses& GetInstance();

		void Initialize(const int32_t& resolutionX, const int32_t& resolutionY, ID3D12Device* device, const D3D12_VIEWPORT& _sceneViewport,
			const D3D12_RECT& _sceneScissorRect, ID3D12GraphicsCommandList* commandList);
		void EnableHDR(ID3D12GraphicsCommandList* commandList, size_t bufferIndex);

	private:
		PostProcesses();
		~PostProcesses() {}

		PostProcesses(const PostProcesses&) = delete;
		PostProcesses(PostProcesses&&) = delete;
		PostProcesses& operator=(const PostProcesses&) = delete;
		PostProcesses& operator=(PostProcesses&&) = delete;

		ConstantBufferId hdrConstantBufferId;

		TextureId noiseTextureId;
		TextureId diffuseTextureId;

		std::shared_ptr<Mesh> screenQuadMesh;
		std::shared_ptr<Material> hdrPostProcessMaterial;
		std::shared_ptr<GraphicObject> hdrPostProcess;

		D3D12_VIEWPORT sceneViewport;
		D3D12_RECT sceneScissorRect;

		using HDRConstantBuffer = struct
		{
			float3 shiftVector;
			float elapsedTime;
			float middleGray;
			float whiteCutoff;
			float brightPassThreshold;
			float brightPassOffset;
		};

		HDRConstantBuffer hdrConstantBuffer;

		ResourceManager& resourceManager = ResourceManager::GetInstance();
	};
}
