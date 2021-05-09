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

		void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const int32_t& resolutionX, const int32_t& resolutionY,
			const D3D12_VIEWPORT& _sceneViewport, const D3D12_RECT& _sceneScissorRect, const RenderTargetId& _sceneRenderTargetId);
		
		void EnableHDR(float3 shiftVector = float3(1.0f, 1.0f, 1.0f), float middleGray = 0.6f, float whiteCutoff = 0.8f, float brightPassThreshold = 5.0f,
			float brightPassOffset = 10.0f);
		void Compose(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);

		void PresentProcessChain(ID3D12GraphicsCommandList* commandList, const D3D12_CPU_DESCRIPTOR_HANDLE* destRenderTargetDescriptor);

	private:
		PostProcesses();
		~PostProcesses() {}

		PostProcesses(const PostProcesses&) = delete;
		PostProcesses(PostProcesses&&) = delete;
		PostProcesses& operator=(const PostProcesses&) = delete;
		PostProcesses& operator=(PostProcesses&&) = delete;

		void ProcessHDR(ID3D12GraphicsCommandList* commandList, const RenderTargetId& srcRenderTargetId, const D3D12_CPU_DESCRIPTOR_HANDLE* destRenderTargetDescriptor);

		ConstantBufferId hdrConstantBufferId;

		TextureId noiseTextureId;
		TextureId diffuseTextureId;

		RenderTargetId sceneRenderTargetId;

		std::shared_ptr<Mesh> screenQuadMesh;
		std::shared_ptr<Material> hdrPostProcessMaterial;
		std::shared_ptr<GraphicObject> hdrPostProcess;

		D3D12_VIEWPORT sceneViewport;
		D3D12_RECT sceneScissorRect;

		bool isHDREnabled;
		bool isComposed;

		D3D12_CPU_DESCRIPTOR_HANDLE sceneRenderTargetDescriptor;
		RenderTargetId hdrInputRenderTargetId;

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
