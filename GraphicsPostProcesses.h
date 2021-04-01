#pragma once

#include "GraphicsHelper.h"
#include "GraphicsResourceManager.h"

using namespace Microsoft::WRL;

class GraphicsPostProcesses
{
public:
	static GraphicsPostProcesses& GetInstance();

	void Initialize(const int32_t& resolutionX, const int32_t& resolutionY, ID3D12Device* device, D3D12_VIEWPORT& _sceneViewport, 
		ID3D12GraphicsCommandList* commandList);
	void EnableHDR(ID3D12GraphicsCommandList* commandList, ID3D12DescriptorHeap* outputRenderTargetDescHeap, size_t bufferIndex);

private:
	GraphicsPostProcesses();
	~GraphicsPostProcesses() {}

	GraphicsPostProcesses(const GraphicsPostProcesses&) = delete;
	GraphicsPostProcesses(GraphicsPostProcesses&&) = delete;
	GraphicsPostProcesses& operator=(const GraphicsPostProcesses&) = delete;
	GraphicsPostProcesses& operator=(GraphicsPostProcesses&&) = delete;

	ComPtr<ID3D12RootSignature> hdrRootSignature;
	ComPtr<ID3D12PipelineState> hdrPipelineState;
	ComPtr<ID3D12DescriptorHeap> renderTargetDescHeap;

	VertexBufferId screenQuadVertexBufferId;
	ConstantBufferId hdrConstantBufferId;

	TextureId noiseTextureId;

	D3D12_VIEWPORT sceneViewport;
	D3D12_RECT sceneScissorRect;

	const uint32_t RENDER_TARGETS_NUMBER = 1;
	const uint32_t CONST_BUFFERS_NUMBER = 1;
	
	uint32_t renderTargetViewDescriptorSize;

	using HdrConstantBuffer = struct
	{
		XMFLOAT3 shiftVector;
		float elapsedTime;
		float middleGray;
		float whiteCutoff;
		float brightPassThreshold;
		float brightPassOffset;
	};

	HdrConstantBuffer hdrConstantBuffer;

	GraphicsResourceManager& resourceManager = GraphicsResourceManager::GetInstance();
};
