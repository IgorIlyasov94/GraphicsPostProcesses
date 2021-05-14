#pragma once

#include "GraphicsHelper.h"
#include "ResourceManager.h"
#include "GraphicObject.h"
#include "GraphicsSettings.h"

namespace Graphics
{
	class PostProcesses
	{
	public:
		static PostProcesses& GetInstance();

		void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const int32_t& resolutionX, const int32_t& resolutionY, const D3D12_VIEWPORT& _sceneViewport,
			const D3D12_RECT& _sceneScissorRect, const RenderTargetId& _sceneRenderTargetId, const RenderTargetId& _normalRenderTargetId,
			const DepthStencilId& _sceneDepthStencilId);
		
		void EnableAA();
		void EnableHDR(float3 shiftVector = float3(1.0f, 1.0f, 1.0f), float middleGray = 0.6f, float whiteCutoff = 0.8f, float brightPassThreshold = 5.0f,
			float brightPassOffset = 10.0f);
		void Compose(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);

		void UpdateHDR(ID3D12GraphicsCommandList* commandList, float3 shiftVector, float middleGray, float whiteCutoff, float brightPassThreshold, float brightPassOffset);
		void PresentProcessChain(ID3D12GraphicsCommandList* commandList, const D3D12_CPU_DESCRIPTOR_HANDLE* destRenderTargetDescriptor);

	private:
		PostProcesses();
		~PostProcesses() {}

		PostProcesses(const PostProcesses&) = delete;
		PostProcesses(PostProcesses&&) = delete;
		PostProcesses& operator=(const PostProcesses&) = delete;
		PostProcesses& operator=(PostProcesses&&) = delete;

		void ProcessAA(ID3D12GraphicsCommandList* commandList, const RenderTargetId& srcRenderTargetId, const D3D12_CPU_DESCRIPTOR_HANDLE* destRenderTargetDescriptor);
		void ProcessHDR(ID3D12GraphicsCommandList* commandList, const RenderTargetId& srcRenderTargetId, const D3D12_CPU_DESCRIPTOR_HANDLE* destRenderTargetDescriptor);

		ConstantBufferId hdrConstantBufferId;

		TextureId noiseTextureId;
		TextureId diffuseTextureId;

		static const uint32_t INTERMEDIATE_8B_RENDER_TARGET_COUNT = 2;
		static const uint32_t INTERMEDIATE_8B_HALF_RENDER_TARGET_COUNT = 2;
		static const uint32_t INTERMEDIATE_8B_QUART_RENDER_TARGET_COUNT = 2;
		static const uint32_t INTERMEDIATE_16B_RENDER_TARGET_COUNT = 4;
		static const uint32_t INTERMEDIATE_32B_RENDER_TARGET_COUNT = 2;

		RenderTargetId sceneRenderTargetId;
		RenderTargetId normalRenderTargetId;
		RenderTargetId intermediate8bTargetId[INTERMEDIATE_8B_RENDER_TARGET_COUNT];
		RenderTargetId intermediate8bHalfTargetId[INTERMEDIATE_8B_HALF_RENDER_TARGET_COUNT];
		RenderTargetId intermediate8bQuartTargetId[INTERMEDIATE_8B_QUART_RENDER_TARGET_COUNT];
		RenderTargetId intermediate16bTargetId[INTERMEDIATE_16B_RENDER_TARGET_COUNT];
		RenderTargetId intermediate32bTargetId[INTERMEDIATE_32B_RENDER_TARGET_COUNT];

		RenderTargetId aaSrcRenderTargetId;
		RenderTargetId hdrSrcRenderTargetId;

		DepthStencilId sceneDepthStencilId;

		D3D12_CPU_DESCRIPTOR_HANDLE sceneRenderTargetDescriptor;
		D3D12_CPU_DESCRIPTOR_HANDLE normalRenderTargetDescriptor;
		D3D12_CPU_DESCRIPTOR_HANDLE sceneDepthStencilDescriptor;
		D3D12_CPU_DESCRIPTOR_HANDLE intermediate8bTargetDescriptor[INTERMEDIATE_8B_RENDER_TARGET_COUNT];
		D3D12_CPU_DESCRIPTOR_HANDLE intermediate8bHalfTargetDescriptor[INTERMEDIATE_8B_HALF_RENDER_TARGET_COUNT];
		D3D12_CPU_DESCRIPTOR_HANDLE intermediate8bQuartTargetDescriptor[INTERMEDIATE_8B_QUART_RENDER_TARGET_COUNT];
		D3D12_CPU_DESCRIPTOR_HANDLE intermediate16bTargetDescriptor[INTERMEDIATE_16B_RENDER_TARGET_COUNT];
		D3D12_CPU_DESCRIPTOR_HANDLE intermediate32bTargetDescriptor[INTERMEDIATE_32B_RENDER_TARGET_COUNT];

		D3D12_CPU_DESCRIPTOR_HANDLE* aaDestRenderTargetDescriptor;
		D3D12_CPU_DESCRIPTOR_HANDLE* hdrDestRenderTargetDescriptor;

		D3D12_CPU_DESCRIPTOR_HANDLE* finalDestRenderTargetDescriptor;

		std::shared_ptr<Mesh> screenQuadMesh;
		std::shared_ptr<Material> antiAliasingMaterial;
		std::shared_ptr<Material> gaussianBlurXMaterial;
		std::shared_ptr<Material> gaussianBlurYMaterial;
		std::shared_ptr<Material> brightPassMaterial;
		std::shared_ptr<Material> toneMappingMaterial;
		std::shared_ptr<GraphicObject> antiAliasing;
		std::shared_ptr<GraphicObject> gaussianBlurX;
		std::shared_ptr<GraphicObject> gaussianBlurY;
		std::shared_ptr<GraphicObject> brightPass;
		std::shared_ptr<GraphicObject> toneMapping;

		D3D12_VIEWPORT sceneViewport;
		D3D12_VIEWPORT sceneViewportHalf;
		D3D12_RECT sceneScissorRect;
		D3D12_RECT sceneScissorRectHalf;

		bool isAAEnabled;
		bool isHDREnabled;
		bool isComposed;

		using AAConstantBuffer = struct
		{
			float2 pixelSize;
			float2 padding;
		};

		using HDRConstantBuffer = struct
		{
			float3 shiftVector;
			float elapsedTime;
			float middleGray;
			float whiteCutoff;
			float brightPassThreshold;
			float brightPassOffset;
		};

		AAConstantBuffer aaConstantBuffer;
		HDRConstantBuffer hdrConstantBuffer;

		ResourceManager& resourceManager = ResourceManager::GetInstance();
	};
}
