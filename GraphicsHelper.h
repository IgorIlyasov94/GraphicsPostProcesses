#pragma once

#include "stdafx.h"

using namespace Microsoft::WRL;
using namespace DirectX;

using ScreenQuadVertex = struct
{
	XMFLOAT3 position;
	XMFLOAT2 texCoord;
};

void CreateRootSignature(ID3D12Device* device, ID3D12RootSignature** rootSignature, D3D12_ROOT_SIGNATURE_FLAGS flags);

void CreateGraphicsPipelineState(ID3D12Device* device, D3D12_INPUT_LAYOUT_DESC&& inputLayoutDesc, ID3D12RootSignature* rootSignature,
	D3D12_RASTERIZER_DESC& rasterizerDesc, D3D12_BLEND_DESC& blendDesc, D3D12_DEPTH_STENCIL_DESC& depthStencilDesc,
	DXGI_FORMAT rtvFormat, D3D12_SHADER_BYTECODE&& vertexShader, D3D12_SHADER_BYTECODE&& pixelShader, ID3D12PipelineState** pipelineState);

void CreateVertexBuffer(ID3D12Device* device, uint8_t* data, uint32_t dataSize, uint32_t dataStride,
	D3D12_VERTEX_BUFFER_VIEW& vertexBufferView, ID3D12Resource** vertexBuffer, ID3D12Resource** vertexBufferUpload);

void SetupRasterizerDesc(D3D12_RASTERIZER_DESC& rasterizerDesc, D3D12_CULL_MODE cullMode = D3D12_CULL_MODE_NONE) noexcept;

void SetupBlendDesc(D3D12_BLEND_DESC& blendDesc, bool blendOn = false,
	D3D12_BLEND srcBlend = D3D12_BLEND_ONE, D3D12_BLEND destBlend = D3D12_BLEND_ZERO, D3D12_BLEND_OP blendOp = D3D12_BLEND_OP_ADD,
	D3D12_BLEND srcBlendAlpha = D3D12_BLEND_ONE, D3D12_BLEND destBlendAlpha = D3D12_BLEND_ZERO, D3D12_BLEND_OP blendOpAlpha = D3D12_BLEND_OP_ADD) noexcept;

void SetupDepthStencilDesc(D3D12_DEPTH_STENCIL_DESC& depthStencilDesc, bool depthEnable) noexcept;
void SetupResourceBufferDesc(D3D12_RESOURCE_DESC& resourceDesc, uint64_t bufferSize,
	D3D12_RESOURCE_FLAGS resourceFlag = D3D12_RESOURCE_FLAG_NONE, uint64_t alignment = 0) noexcept;

void SetupHeapProperties(D3D12_HEAP_PROPERTIES& heapProperties, D3D12_HEAP_TYPE heapType) noexcept;