#pragma once

#include "GraphicsHelper.h"
#include "ResourceManager.h"

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

		ComPtr<ID3D12RootSignature> hdrRootSignature;
		ComPtr<ID3D12PipelineState> hdrPipelineState;
		ComPtr<ID3D12DescriptorHeap> renderTargetDescHeap;

		VertexBufferId screenQuadVertexBufferId;
		ConstantBufferId hdrConstantBufferId;

		TextureId noiseTextureId;
		TextureId diffuseTextureId;

		D3D12_VIEWPORT sceneViewport;
		D3D12_RECT sceneScissorRect;

		const uint32_t RENDER_TARGETS_NUMBER = 1;
		const uint32_t CONST_BUFFERS_NUMBER = 1;

		uint32_t renderTargetViewDescriptorSize;

		using HdrConstantBuffer = struct
		{
			float3 shiftVector;
			float elapsedTime;
			float middleGray;
			float whiteCutoff;
			float brightPassThreshold;
			float brightPassOffset;
		};

		HdrConstantBuffer hdrConstantBuffer;

		ResourceManager& resourceManager = ResourceManager::GetInstance();
	};
}
