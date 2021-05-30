#pragma once

#include "ResourceManager.h"

namespace Graphics
{
	class ComputeObject
	{
	public:
		ComputeObject();
		~ComputeObject();

		ConstantBufferId SetConstantBuffer(size_t registerIndex, void* bufferData, size_t bufferSize);
		
		void AssignConstantBuffer(size_t registerIndex, ConstantBufferId constantBufferId);
		void AssignTexture(ID3D12GraphicsCommandList* commandList, size_t registerIndex, TextureId textureId);
		void AssignTexture(size_t registerIndex, RWTextureId rwTextureId);
		void AssignRenderTexture(size_t registerIndex, RenderTargetId renderTargetId);
		void AssignDepthTexture(size_t registerIndex, DepthStencilId depthStencilId);
		void AssignBuffer(size_t registerIndex, BufferId bufferId);
		void AssignBuffer(size_t registerIndex, RWBufferId rwBufferId);
		void AssignRWTexture(size_t registerIndex, RWTextureId rwTextureId);
		void AssignRWBuffer(size_t registerIndex, RWBufferId rwBufferId);

		void AssignShader(D3D12_SHADER_BYTECODE _computeShader);

		void UpdateConstantBuffer(ConstantBufferId constantBufferId, void* bufferData, size_t bufferSize);

		void Compose(ID3D12Device* device);

		bool IsComposed() const noexcept;

		void SetThreadGroupCount(uint32_t xCount, uint32_t yCount, uint32_t zCount);

		void Present(ID3D12GraphicsCommandList* commandList) const;

	private:
		using RegisterSet = struct
		{
			std::vector<size_t> constantBufferRegisterIndices;
			std::vector<size_t> textureRegisterIndices;
			std::vector<size_t> bufferRegisterIndices;
			std::vector<size_t> renderTextureRegisterIndices;
			std::vector<size_t> depthTextureRegisterIndices;
			std::vector<size_t> rwTextureReadOnlyRegisterIndices;
			std::vector<size_t> rwBufferReadOnlyRegisterIndices;
			std::vector<size_t> rwTextureRegisterIndices;
			std::vector<size_t> rwBufferRegisterIndices;
		};

		using IndexSet = struct
		{
			std::vector<ConstantBufferId> constantBufferIndices;
			std::vector<TextureId> textureIndices;
			std::vector<BufferId> bufferIndices;
			std::vector<RenderTargetId> renderTargetIndices;
			std::vector<DepthStencilId> depthStencilIndices;
			std::vector<RWTextureId> rwTextureReadOnlyIndices;
			std::vector<RWBufferId> rwBufferReadOnlyIndices;
			std::vector<RWTextureId> rwTextureIndices;
			std::vector<RWBufferId> rwBufferIndices;
		};

		void CreateResourceRootDescriptorTables(const RegisterSet& _registerSet, std::vector<D3D12_DESCRIPTOR_RANGE>& descriptorRanges,
			std::vector<D3D12_ROOT_DESCRIPTOR_TABLE>& rootDescriptorTables);

		void CreateRootParameters(const RegisterSet& _registerSet, const std::vector<D3D12_ROOT_DESCRIPTOR_TABLE>& rootDescriptorTables,
			std::vector<D3D12_ROOT_PARAMETER>& rootParameters);

		void CreateRootSignature(ID3D12Device* device, const std::vector<D3D12_ROOT_PARAMETER>& rootParameters, ID3D12RootSignature** rootSignature);

		void CreateGraphicsPipelineState(ID3D12Device* device, ID3D12RootSignature* rootSignature, D3D12_SHADER_BYTECODE _computeShader, ID3D12PipelineState** pipelineState);

		D3D12_SHADER_BYTECODE computeShader;

		RegisterSet registerSet;
		IndexSet indexSet;

		std::vector<D3D12_GPU_VIRTUAL_ADDRESS> constantBufferAddresses;
		
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> firstResourceDescriptorBases;
		
		ComPtr<ID3D12RootSignature> rootSignature;
		ComPtr<ID3D12PipelineState> pipelineState;

		uint32_t threadGroupCountX;
		uint32_t threadGroupCountY;
		uint32_t threadGroupCountZ;

		bool isComposed;

		ResourceManager& resourceManager = ResourceManager::GetInstance();
	};
}
