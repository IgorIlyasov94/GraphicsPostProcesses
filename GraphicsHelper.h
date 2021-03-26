#pragma once

#include "stdafx.h"

using namespace Microsoft::WRL;
using namespace DirectX;

using ScreenQuadVertex = struct
{
	XMFLOAT3 position;
	XMFLOAT2 texCoord;
};

constexpr size_t _KB = 1024;
constexpr size_t _MB = 1024 * _KB;
constexpr size_t _GB = 1024 * _MB;

void CreateFactory(IDXGIFactory4** _factory);
void CreateDevice(IDXGIAdapter1* adapter, ID3D12Device** _device);
void CreateCommandQueue(ID3D12Device* _device, ID3D12CommandQueue** _commandQueue);

void CreateSwapChain(IDXGIFactory4* _factory, ID3D12CommandQueue* _commandQueue, HWND& _windowHandler, const uint32_t buffersCount,
	const int32_t& _resolutionX, const int32_t& _resolutionY, IDXGISwapChain1** _swapChain);

void CreateRootSignature(ID3D12Device* device, ID3D12RootSignature** rootSignature, D3D12_ROOT_SIGNATURE_FLAGS flags);

void CreateDescriptorHeap(ID3D12Device* device, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS flags, D3D12_DESCRIPTOR_HEAP_TYPE type,
	ID3D12DescriptorHeap** descriptorHeap);

void CreateGraphicsPipelineState(ID3D12Device* device, D3D12_INPUT_LAYOUT_DESC&& inputLayoutDesc, ID3D12RootSignature* rootSignature,
	D3D12_RASTERIZER_DESC& rasterizerDesc, D3D12_BLEND_DESC& blendDesc, D3D12_DEPTH_STENCIL_DESC& depthStencilDesc,
	DXGI_FORMAT rtvFormat, D3D12_SHADER_BYTECODE&& vertexShader, D3D12_SHADER_BYTECODE&& pixelShader, ID3D12PipelineState** pipelineState);

/*void CreateVertexBuffer(ID3D12Device* device, uint8_t* data, uint32_t dataSize, uint32_t dataStride, D3D12_VERTEX_BUFFER_VIEW& vertexBufferView,
	ID3D12Resource** vertexBuffer, ID3D12Resource** vertexBufferUpload, ID3D12GraphicsCommandList* commandList);

void CreateConstantBuffer(ID3D12Device* device, std::vector<uint8_t*>& data, std::vector<uint64_t>& bufferSizes, ID3D12DescriptorHeap* descriptorHeap,
	ID3D12Resource** constantBuffer, ID3D12GraphicsCommandList* commandList);*/

void GetHardwareAdapter(IDXGIFactory4* factory4, IDXGIAdapter1** adapter);

void SetupRasterizerDesc(D3D12_RASTERIZER_DESC& rasterizerDesc, D3D12_CULL_MODE cullMode = D3D12_CULL_MODE_NONE) noexcept;

void SetupBlendDesc(D3D12_BLEND_DESC& blendDesc, bool blendOn = false,
	D3D12_BLEND srcBlend = D3D12_BLEND_ONE, D3D12_BLEND destBlend = D3D12_BLEND_ZERO, D3D12_BLEND_OP blendOp = D3D12_BLEND_OP_ADD,
	D3D12_BLEND srcBlendAlpha = D3D12_BLEND_ONE, D3D12_BLEND destBlendAlpha = D3D12_BLEND_ZERO, D3D12_BLEND_OP blendOpAlpha = D3D12_BLEND_OP_ADD) noexcept;

void SetupDepthStencilDesc(D3D12_DEPTH_STENCIL_DESC& depthStencilDesc, bool depthEnable) noexcept;
void SetupResourceBufferDesc(D3D12_RESOURCE_DESC& resourceDesc, uint64_t bufferSize,
	D3D12_RESOURCE_FLAGS resourceFlag = D3D12_RESOURCE_FLAG_NONE, uint64_t alignment = 0) noexcept;

void SetupHeapProperties(D3D12_HEAP_PROPERTIES& heapProperties, D3D12_HEAP_TYPE heapType) noexcept;

void SetResourceBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* const resource, D3D12_RESOURCE_STATES resourceBarrierStateBefore,
	D3D12_RESOURCE_STATES resourceBarrierStateAfter, D3D12_RESOURCE_BARRIER_TYPE resourceBarrierType = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION);

template<typename T>
inline const T AlignSize(const T size, const T alignment)
{
	return (size + alignment - 1) & ~(alignment - 1);
}
