#pragma once

#include "stdafx.h"

using namespace Microsoft::WRL;

void CreateRootSignature(ID3D12Device* device, ID3D12RootSignature* rootSignature);
void CreateVertexShader(LPCWSTR shaderPath, ID3DBlob* vertexShader);
void CreatePixelShader(LPCWSTR shaderPath, ID3DBlob* pixelShader);

void CreateGraphicsPipelineState(ID3D12Device* device, D3D12_INPUT_LAYOUT_DESC& inputLayoutDesc, ID3D12RootSignature* rootSignature,
	D3D12_RASTERIZER_DESC& rasterizerDesc, D3D12_BLEND_DESC& blendDesc, D3D12_DEPTH_STENCIL_DESC& depthStencilDesc,
	DXGI_FORMAT rtvFormat, ID3DBlob* vertexShader, ID3DBlob* pixelShader, ID3D12PipelineState* pipelineState);

void SetupRasterizerDesc(D3D12_RASTERIZER_DESC& rasterizerDesc, D3D12_CULL_MODE cullMode = D3D12_CULL_MODE_NONE);

void SetupBlendDesc(D3D12_BLEND_DESC& blendDesc, bool blendOn = false,
	D3D12_BLEND srcBlend = D3D12_BLEND_ONE, D3D12_BLEND destBlend = D3D12_BLEND_ZERO, D3D12_BLEND_OP blendOp = D3D12_BLEND_OP_ADD,
	D3D12_BLEND srcBlendAlpha = D3D12_BLEND_ONE, D3D12_BLEND destBlendAlpha = D3D12_BLEND_ZERO, D3D12_BLEND_OP blendOpAlpha = D3D12_BLEND_OP_ADD);

void SetupDepthStencilDesc(D3D12_DEPTH_STENCIL_DESC& depthStencilDesc, bool depthEnable);