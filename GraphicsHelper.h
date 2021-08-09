#pragma once

#include "stdafx.h"
#include "RingBuffer.h"

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

	enum class UIHorizontalAlign
	{
		UI_ALIGN_LEFT,
		UI_ALIGN_CENTER,
		UI_ALIGN_RIGHT
	};

	enum class UIVerticalAlign
	{
		UI_ALIGN_TOP,
		UI_ALIGN_MIDDLE,
		UI_ALIGN_BOTTOM
	};

	constexpr size_t _KB = 1024;
	constexpr size_t _MB = 1024 * _KB;
	constexpr size_t _GB = 1024 * _MB;

	void CreateFactory(IDXGIFactory4** _factory);
	void CreateDevice(IDXGIAdapter1* adapter, ID3D12Device** _device);
	void CreateCommandQueue(ID3D12Device* _device, ID3D12CommandQueue** _commandQueue);

	void CreateSwapChain(IDXGIFactory4* _factory, ID3D12CommandQueue* _commandQueue, HWND& _windowHandler, const uint32_t buffersCount,
		const int32_t& _resolutionX, const int32_t& _resolutionY, IDXGISwapChain1** _swapChain);

	void CreateDescriptorHeap(ID3D12Device* device, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS flags, D3D12_DESCRIPTOR_HEAP_TYPE type,
		ID3D12DescriptorHeap** descriptorHeap);

	void ReadShaderConstantBuffers(const D3D12_SHADER_BYTECODE& shaderBytecode, std::set<size_t>& constantBufferIndices);

	void CreateStandardSamplerDescs(std::vector<D3D12_STATIC_SAMPLER_DESC>& samplerDescs);

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

	bool CheckBoxInBox(const BoundingBox& sourceBox, const BoundingBox& destinationBox) noexcept;
	bool CheckBoxInBox(const float3& sourceBoxSize, const float3& destinationBoxSize) noexcept;
	BoundingBox ExpandBoundingBox(const BoundingBox& targetBox, const BoundingBox& appendableBox) noexcept;

	float3 BoundingBoxSize(const BoundingBox& boundingBox);
	float BoundingBoxVolume(const BoundingBox& boundingBox);
	void BoundingBoxVertices(const BoundingBox& boundingBox, std::array<floatN, 8>& vertices);
	
	template<typename T>
	constexpr T AlignSize(const T size, const T alignment) noexcept
	{
		return (size + alignment - 1) & ~(alignment - 1);
	}
}
