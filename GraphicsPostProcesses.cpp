#include "GraphicsPostProcesses.h"
#include "Resources/Shaders/ScreenQuad.vsh.h"
#include "Resources/Shaders/HDRToneMapping.psh.h"

GraphicsPostProcesses::GraphicsPostProcesses()
	: screenQuadVertexBufferView{}, sceneViewport{},
	renderTargetViewDescriptorSize(0), shaderResourceViewDescriptorSize(0)
{

}

GraphicsPostProcesses& GraphicsPostProcesses::GetInstance()
{
	static GraphicsPostProcesses instance;

	return instance;
}

void GraphicsPostProcesses::Initialize(const int32_t& resolutionX, const int32_t& resolutionY, ID3D12Device* device, D3D12_VIEWPORT& _sceneViewport,
	ID3D12GraphicsCommandList* commandList)
{
	sceneViewport = _sceneViewport;
	sceneScissorRect.left = 0;
	sceneScissorRect.top = 0;
	sceneScissorRect.right = static_cast<long>(sceneViewport.Width);
	sceneScissorRect.bottom = static_cast<long>(sceneViewport.Height);

	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	auto rootFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | 
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	CreateRootSignature(device, &hdrRootSignature, rootFlags);

	D3D12_RASTERIZER_DESC rasterizerDesc;
	SetupRasterizerDesc(rasterizerDesc, D3D12_CULL_MODE_NONE);

	D3D12_BLEND_DESC blendDesc;
	SetupBlendDesc(blendDesc);

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	SetupDepthStencilDesc(depthStencilDesc, false);

	CreateGraphicsPipelineState(device, { inputElementDescs , _countof(inputElementDescs) }, hdrRootSignature.Get(),
		rasterizerDesc, blendDesc, depthStencilDesc, DXGI_FORMAT_R8G8B8A8_UNORM, { quadVertexShader, sizeof(quadVertexShader) },
		{ toneMappingPixelShader, sizeof(toneMappingPixelShader) }, &hdrPipelineState);

	ScreenQuadVertex vertices[] =
	{
		{ XMFLOAT3(-1.0f, 1.0f, 0.5f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 0.5f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 0.5f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 0.5f), XMFLOAT2(1.0f, 1.0f) }
	};

	CreateVertexBuffer(device, reinterpret_cast<uint8_t*>(vertices), sizeof(vertices), sizeof(ScreenQuadVertex),
		screenQuadVertexBufferView, &screenQuadVertexBuffer, &screenQuadVertexBufferUpload);

	renderTargetViewDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	D3D12_RESOURCE_BARRIER vertexBufferResourceBarrier{};
	vertexBufferResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	vertexBufferResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	vertexBufferResourceBarrier.Transition.pResource = screenQuadVertexBuffer.Get();
	vertexBufferResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	vertexBufferResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
	vertexBufferResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	commandList->ResourceBarrier(1, &vertexBufferResourceBarrier);
	commandList->CopyBufferRegion(screenQuadVertexBuffer.Get(), 0, screenQuadVertexBufferUpload.Get(), 0, screenQuadVertexBuffer->GetDesc().Width);
	
	vertexBufferResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	vertexBufferResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	
	commandList->ResourceBarrier(1, &vertexBufferResourceBarrier);
}

void GraphicsPostProcesses::EnableHDR(ID3D12GraphicsCommandList* commandList, ID3D12DescriptorHeap* outputRenderTargetDescHeap, size_t outputRenderTargetOffset)
{
	commandList->SetPipelineState(hdrPipelineState.Get());

	commandList->SetGraphicsRootSignature(hdrRootSignature.Get());
	
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	commandList->IASetVertexBuffers(0, 1, &screenQuadVertexBufferView);
	commandList->RSSetViewports(1, &sceneViewport);
	commandList->RSSetScissorRects(1, &sceneScissorRect);
	
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetHandle(outputRenderTargetDescHeap->GetCPUDescriptorHandleForHeapStart());
	renderTargetHandle.ptr += outputRenderTargetOffset;

	commandList->OMSetRenderTargets(1, &renderTargetHandle, false, nullptr);

	commandList->DrawInstanced(4, 1, 0, 0);
}