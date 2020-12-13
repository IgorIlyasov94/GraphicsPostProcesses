#pragma once

#include "GraphicsHelper.h"

using namespace Microsoft::WRL;

class GraphicsPostProcesses
{
public:
	static GraphicsPostProcesses& GetInstance();

	void Initialize(const int32_t& resolutionX, const int32_t& resolutionY, ID3D12Device* device);
	void EnableHDR();

private:
	GraphicsPostProcesses();
	~GraphicsPostProcesses() {}

	GraphicsPostProcesses(const GraphicsPostProcesses&) = delete;
	GraphicsPostProcesses(GraphicsPostProcesses&&) = delete;
	GraphicsPostProcesses& operator=(const GraphicsPostProcesses&) = delete;
	GraphicsPostProcesses& operator=(GraphicsPostProcesses&&) = delete;

	ComPtr<ID3D12RootSignature> hdrRootSignature;
	ComPtr<ID3D12PipelineState> hdrPipelineState;
	ComPtr<ID3D12Resource> screenQuadVertexBuffer;
	ComPtr<ID3D12Resource> screenQuadVertexBufferUpload;

	D3D12_VERTEX_BUFFER_VIEW screenQuadVertexBufferView;
};