#pragma once

#include "stdafx.h"

using namespace Microsoft::WRL;
using namespace DirectX;

using floatN = XMVECTOR;
using float4 = XMFLOAT4;
using float3 = XMFLOAT3;
using float2 = XMFLOAT2;
using float4x4 = XMMATRIX;

namespace Graphics
{
	
	using ScreenQuadVertex = struct
	{
		float3 position;
		float2 texCoord;
	};

	using ShaderList = struct
	{
		D3D12_SHADER_BYTECODE vertexShader;
		D3D12_SHADER_BYTECODE hullShader;
		D3D12_SHADER_BYTECODE domainShader;
		D3D12_SHADER_BYTECODE geometryShader;
		D3D12_SHADER_BYTECODE pixelShader;
	};

	using TextureInfo = struct
	{
		uint64_t width;
		uint32_t height;
		uint16_t depth;
		uint16_t mipLevels;
		uint64_t rowPitch;
		uint64_t slicePitch;
		DXGI_FORMAT format;
		D3D12_RESOURCE_DIMENSION dimension;
		D3D12_SRV_DIMENSION srvDimension;
	};

	using BoundingBox = struct
	{
		float3 minCornerPoint;
		float3 maxCornerPoint;
	};

	using BoundingSphere = struct
	{
		float3 center;
		float radius;
	};

	enum class VertexFormat
	{
		UNDEFINED,
		POSITION,
		POSITION_TEXCOORD,
		POSITION_NORMAL,
		POSITION_NORMAL_TANGENT_BINORMAL,
		POSITION_NORMAL_TEXCOORD,
		POSITION_NORMAL_TANGENT_BINORMAL_TEXCOORD
	};

	constexpr size_t _KB = 1024;
	constexpr size_t _MB = 1024 * _KB;
	constexpr size_t _GB = 1024 * _MB;

	void CreateFactory(IDXGIFactory4** _factory);
	void CreateDevice(IDXGIAdapter1* adapter, ID3D12Device** _device);
	void CreateCommandQueue(ID3D12Device* _device, ID3D12CommandQueue** _commandQueue);

	void CreateSwapChain(IDXGIFactory4* _factory, ID3D12CommandQueue* _commandQueue, HWND& _windowHandler, const uint32_t buffersCount,
		const int32_t& _resolutionX, const int32_t& _resolutionY, IDXGISwapChain1** _swapChain);

	void CreateRootSignature(ID3D12Device* device, const std::vector<D3D12_ROOT_PARAMETER>& rootParameters, const std::vector<D3D12_STATIC_SAMPLER_DESC>& samplerDescs,
		D3D12_ROOT_SIGNATURE_FLAGS flags, ID3D12RootSignature** rootSignature);

	void CreateDescriptorHeap(ID3D12Device* device, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS flags, D3D12_DESCRIPTOR_HEAP_TYPE type,
		ID3D12DescriptorHeap** descriptorHeap);

	void CreateGraphicsPipelineState(ID3D12Device* device, const D3D12_INPUT_LAYOUT_DESC& inputLayoutDesc, ID3D12RootSignature* rootSignature,
		const D3D12_RASTERIZER_DESC& rasterizerDesc, const D3D12_BLEND_DESC& blendDesc, const D3D12_DEPTH_STENCIL_DESC& depthStencilDesc,
		const std::array<DXGI_FORMAT, 8>& rtvFormat, DXGI_FORMAT dsvFormat, const ShaderList& shaderList, ID3D12PipelineState** pipelineState);

	void ReadShaderConstantBuffers(const D3D12_SHADER_BYTECODE& shaderBytecode, std::set<size_t>& constantBufferIndices);

	void CreateRootParameters(const ShaderList& shaderList, const std::vector<size_t>& constantBufferRegisterIndices, const D3D12_ROOT_DESCRIPTOR_TABLE& rootDescriptorTable,
		D3D12_ROOT_SIGNATURE_FLAGS& rootSignatureFlags, std::vector<D3D12_ROOT_PARAMETER>& rootParameters);
	void CreateTextureRootDescriptorTable(const std::vector<size_t>& textureRegisterIndices, const std::vector<size_t>& descriptorIndices,
		std::vector<D3D12_DESCRIPTOR_RANGE>& descriptorRange, D3D12_ROOT_DESCRIPTOR_TABLE& rootDescriptorTable);
	void CreateStandardSamplerDescs(std::vector<D3D12_STATIC_SAMPLER_DESC>& samplerDescs);

	void CreatePipelineStateAndRootSignature(ID3D12Device* device, const D3D12_INPUT_LAYOUT_DESC& inputLayoutDesc, const D3D12_RASTERIZER_DESC& rasterizerDesc,
		const D3D12_BLEND_DESC& blendDesc, const D3D12_DEPTH_STENCIL_DESC& depthStencilDesc, const std::array<DXGI_FORMAT, 8>& rtvFormat, DXGI_FORMAT dsvFormat,
		const ShaderList& shaderList, const std::vector<size_t>& constantBufferIndices, const D3D12_ROOT_DESCRIPTOR_TABLE& texturesRootDescriptorTable,
		const std::vector<D3D12_STATIC_SAMPLER_DESC>& samplerDescs, ID3D12RootSignature** rootSignature, ID3D12PipelineState** pipelineState);

	void GetHardwareAdapter(IDXGIFactory4* factory4, IDXGIAdapter1** adapter);
	
	void SetupRasterizerDesc(D3D12_RASTERIZER_DESC& rasterizerDesc, D3D12_CULL_MODE cullMode = D3D12_CULL_MODE_NONE) noexcept;

	void SetupBlendDesc(D3D12_BLEND_DESC& blendDesc, bool blendOn = false,
		D3D12_BLEND srcBlend = D3D12_BLEND_ONE, D3D12_BLEND destBlend = D3D12_BLEND_ZERO, D3D12_BLEND_OP blendOp = D3D12_BLEND_OP_ADD,
		D3D12_BLEND srcBlendAlpha = D3D12_BLEND_ONE, D3D12_BLEND destBlendAlpha = D3D12_BLEND_ZERO, D3D12_BLEND_OP blendOpAlpha = D3D12_BLEND_OP_ADD) noexcept;

	void SetupDepthStencilDesc(D3D12_DEPTH_STENCIL_DESC& depthStencilDesc, bool depthEnable) noexcept;
	void SetupResourceBufferDesc(D3D12_RESOURCE_DESC& resourceDesc, uint64_t bufferSize, D3D12_RESOURCE_FLAGS resourceFlag = D3D12_RESOURCE_FLAG_NONE,
		uint64_t alignment = 0) noexcept;

	void SetupResourceTextureDesc(D3D12_RESOURCE_DESC& resourceDesc, const TextureInfo& textureInfo, D3D12_RESOURCE_FLAGS resourceFlag = D3D12_RESOURCE_FLAG_NONE,
		uint64_t alignment = 0) noexcept;

	void SetupHeapProperties(D3D12_HEAP_PROPERTIES& heapProperties, D3D12_HEAP_TYPE heapType) noexcept;

	void SetResourceBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* const resource, D3D12_RESOURCE_STATES resourceBarrierStateBefore,
		D3D12_RESOURCE_STATES resourceBarrierStateAfter, D3D12_RESOURCE_BARRIER_TYPE resourceBarrierType = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION);

	bool CheckBoxInBox(const BoundingBox& sourceBox, const BoundingBox& destinationBox) noexcept;
	bool CheckBoxInBox(const float3& sourceBoxSize, const float3& destinationBoxSize) noexcept;
	BoundingBox ExpandBoundingBox(const BoundingBox& targetBox, const BoundingBox& appendableBox) noexcept;

	float3 BoundingBoxSize(const BoundingBox& boundingBox);
	float BoundingBoxVolume(const BoundingBox& boundingBox);
	void BoundingBoxVertices(const BoundingBox& boundingBox, std::array<floatN, 8>& vertices);
	
	constexpr uint32_t GetVertexStride(VertexFormat vertexFormat) noexcept
	{
		if (vertexFormat == VertexFormat::POSITION)
			return 12;
		else if (vertexFormat == VertexFormat::POSITION_TEXCOORD)
			return 20;
		else if (vertexFormat == VertexFormat::POSITION_NORMAL)
			return 24;
		else if (vertexFormat == VertexFormat::POSITION_NORMAL_TEXCOORD)
			return 32;
		else if (vertexFormat == VertexFormat::POSITION_NORMAL_TANGENT_BINORMAL)
			return 48;
		else if (vertexFormat == VertexFormat::POSITION_NORMAL_TANGENT_BINORMAL_TEXCOORD)
			return 56;

		return 0;
	}

	template<typename T>
	constexpr T AlignSize(const T size, const T alignment) noexcept
	{
		return (size + alignment - 1) & ~(alignment - 1);
	}
}
