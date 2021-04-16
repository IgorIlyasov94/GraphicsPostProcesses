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

		void Initialize(const int32_t& resolutionX, const int32_t& resolutionY, ID3D12Device* device, D3D12_VIEWPORT& _sceneViewport,
			ID3D12GraphicsCommandList* commandList);
		void EnableHDR(ID3D12GraphicsCommandList* commandList, ID3D12DescriptorHeap* outputRenderTargetDescHeap, size_t bufferIndex);

	private:
		PostProcesses();
		~PostProcesses() {}

		PostProcesses(const PostProcesses&) = delete;
		PostProcesses(PostProcesses&&) = delete;
		PostProcesses& operator=(const PostProcesses&) = delete;
		PostProcesses& operator=(PostProcesses&&) = delete;

		ComPtr<ID3D12DescriptorHeap> renderTargetDescHeap;

		ConstantBufferId hdrConstantBufferId;

		TextureId noiseTextureId;
		TextureId diffuseTextureId;

		std::shared_ptr<Mesh> screenQuadMesh;
		std::shared_ptr<Material> hdrPostProcessMaterial;
		std::shared_ptr<GraphicObject> hdrPostProcess;

		D3D12_VIEWPORT sceneViewport;
		D3D12_RECT sceneScissorRect;

		const uint32_t RENDER_TARGETS_NUMBER = 1;
		
		uint32_t renderTargetViewDescriptorSize;

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
