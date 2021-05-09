#pragma once

#include "ResourceManager.h"

namespace Graphics
{
	class Material
	{
	public:
		Material();
		~Material();

		ConstantBufferId SetConstantBuffer(size_t registerIndex, void* bufferData, size_t bufferSize);
		void SetSampler(size_t registerIndex, D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE addressU, D3D12_TEXTURE_ADDRESS_MODE addressV,
			D3D12_TEXTURE_ADDRESS_MODE addressW, uint32_t maxAnisotropy);

		void AssignConstantBuffer(size_t registerIndex, ConstantBufferId constantBufferId);
		void AssignTexture(ID3D12GraphicsCommandList* commandList, size_t registerIndex, TextureId textureId, bool asPixelShaderResource);
		void AssignRenderTexture(ID3D12GraphicsCommandList* commandList, size_t registerIndex, RenderTargetId renderTargetId);

		void SetVertexShader(D3D12_SHADER_BYTECODE shaderBytecode);
		void SetHullShader(D3D12_SHADER_BYTECODE shaderBytecode);
		void SetDomainShader(D3D12_SHADER_BYTECODE shaderBytecode);
		void SetGeometryShader(D3D12_SHADER_BYTECODE shaderBytecode);
		void SetPixelShader(D3D12_SHADER_BYTECODE shaderBytecode);

		void SetVertexFormat(VertexFormat format);
		void SetRenderTargetFormat(size_t renderTargetIndex, DXGI_FORMAT format);
		void SetDepthStencilFormat(uint32_t depthBit);

		void SetDepthTest(bool _useDepthBuffer);
		void SetCullMode(D3D12_CULL_MODE _cullMode);
		void SetBlendMode(bool blendOn, D3D12_BLEND srcBlend = D3D12_BLEND_ONE, D3D12_BLEND destBlend = D3D12_BLEND_ZERO, D3D12_BLEND_OP blendOp = D3D12_BLEND_OP_ADD,
			D3D12_BLEND srcBlendAlpha = D3D12_BLEND_ONE, D3D12_BLEND destBlendAlpha = D3D12_BLEND_ZERO, D3D12_BLEND_OP blendOpAlpha = D3D12_BLEND_OP_ADD);

		void UpdateConstantBuffer(ConstantBufferId constantBufferId, void* bufferData, size_t bufferSize);

		void Compose(ID3D12Device* device);

		bool IsComposed() const noexcept;

		void Present(ID3D12GraphicsCommandList* commandList) const;

	private:
		void CreateInputElementDescs(VertexFormat format, std::vector<D3D12_INPUT_ELEMENT_DESC>& inputElementDescs) const noexcept;
		void CreateTextureRootDescriptorTables(const std::vector<size_t>& textureRegisterIndices, const std::vector<size_t>& renderTextureRegisterIndices,
			std::vector<D3D12_DESCRIPTOR_RANGE>& descriptorRanges, std::vector<D3D12_ROOT_DESCRIPTOR_TABLE>& rootDescriptorTable);

		void CreateRootParameters(const ShaderList& shaderList, const std::vector<size_t>& constantBufferRegisterIndices,
			const std::vector<D3D12_ROOT_DESCRIPTOR_TABLE>& rootDescriptorTables, D3D12_ROOT_SIGNATURE_FLAGS& rootSignatureFlags, std::vector<D3D12_ROOT_PARAMETER>& rootParameters);

		void CreateRootSignature(ID3D12Device* device, const std::vector<D3D12_ROOT_PARAMETER>& rootParameters, const std::vector<D3D12_STATIC_SAMPLER_DESC>& samplerDescs,
			D3D12_ROOT_SIGNATURE_FLAGS flags, ID3D12RootSignature** rootSignature);

		void CreateGraphicsPipelineState(ID3D12Device* device, const D3D12_INPUT_LAYOUT_DESC& inputLayoutDesc, ID3D12RootSignature* rootSignature,
			const D3D12_RASTERIZER_DESC& rasterizerDesc, const D3D12_BLEND_DESC& blendDesc, const D3D12_DEPTH_STENCIL_DESC& depthStencilDesc,
			const std::array<DXGI_FORMAT, 8>& rtvFormat, DXGI_FORMAT dsvFormat, const ShaderList& shaderList, ID3D12PipelineState** pipelineState);

		ShaderList shaderList;

		std::vector<size_t> constantBufferRegisterIndices;
		std::vector<size_t> textureRegisterIndices;
		std::vector<size_t> renderTextureRegisterIndices;

		std::vector<ConstantBufferId> constantBufferIndices;
		std::vector<TextureId> textureIndices;
		std::vector<RenderTargetId> renderTargetIndices;

		std::vector<D3D12_GPU_VIRTUAL_ADDRESS> constantBufferAddresses;
		std::vector<D3D12_STATIC_SAMPLER_DESC> samplerDescs;

		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> firstTextureDescriptorBases;

		VertexFormat vertexFormat;

		std::array<DXGI_FORMAT, 8> renderTargetFormat;
		DXGI_FORMAT depthStencilFormat;

		D3D12_CULL_MODE cullMode;
		D3D12_BLEND_DESC blendDesc;

		bool useDepthBuffer;

		ComPtr<ID3D12RootSignature> rootSignature;
		ComPtr<ID3D12PipelineState> pipelineState;

		bool isComposed;

		ResourceManager& resourceManager = ResourceManager::GetInstance();
	};
}
