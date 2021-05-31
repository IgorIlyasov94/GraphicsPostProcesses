#pragma once

#include "BufferAllocator.h"
#include "DescriptorAllocator.h"
#include "TextureAllocator.h"
#include "DDSLoader.h"

namespace Graphics
{
	template<uint8_t Category>
	class ResourceId
	{
	public:
		ResourceId()
			: value(0), category(Category)
		{

		}

		ResourceId(uint32_t resourceId)
			: value(resourceId), category(Category)
		{

		}

		uint32_t value;

	private:
		friend class ResourceManager;

		uint8_t category;
	};

	using VertexBufferId = typename ResourceId<0>;
	using IndexBufferId = typename ResourceId<1>;
	using ConstantBufferId = typename ResourceId<2>;
	using TextureId = typename ResourceId<3>;
	using BufferId = typename ResourceId<4>;
	using SamplerId = typename ResourceId<5>;
	using RenderTargetId = typename ResourceId<6>;
	using DepthStencilId = typename ResourceId<7>;
	using RWTextureId = typename ResourceId<8>;
	using RWBufferId = typename ResourceId<9>;

	struct VertexBuffer
	{
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		BufferAllocation vertexBufferAllocation;
	};

	struct IndexBuffer
	{
		uint32_t indicesCount;
		D3D12_INDEX_BUFFER_VIEW indexBufferView;
		BufferAllocation indexBufferAllocation;
	};

	struct ConstantBuffer
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC constantBufferViewDesc;
		BufferAllocation uploadBufferAllocation;
		DescriptorAllocation bufferDescriptorAllocation;
	};

	struct Texture
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		TextureInfo info;
		TextureAllocation textureAllocation;
		DescriptorAllocation shaderResourceDescriptorAllocation;
	};

	struct Buffer
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		BufferAllocation bufferAllocation;
		DescriptorAllocation shaderResourceDescriptorAllocation;
	};

	struct Sampler
	{
		D3D12_SAMPLER_DESC samplerDesc;
		DescriptorAllocation samplerDescriptorAllocation;
	};

	struct RenderTarget
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		TextureInfo info;
		TextureAllocation textureAllocation;
		DescriptorAllocation shaderResourceDescriptorAllocation;
		DescriptorAllocation renderTargetDescriptorAllocation;
	};

	struct DepthStencil
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		TextureInfo info;
		TextureAllocation textureAllocation;
		DescriptorAllocation shaderResourceDescriptorAllocation;
		DescriptorAllocation depthStencilDescriptorAllocation;
	};

	struct RWTexture
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		D3D12_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc;
		TextureInfo info;
		TextureAllocation textureAllocation;
		DescriptorAllocation shaderResourceDescriptorAllocation;
		DescriptorAllocation unorderedAccessDescriptorAllocation;
	};

	struct RWBuffer
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		D3D12_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc;
		BufferAllocation bufferAllocation;
		DescriptorAllocation shaderResourceDescriptorAllocation;
		DescriptorAllocation unorderedAccessDescriptorAllocation;
	};

	class ResourceManager
	{
	public:
		static ResourceManager& GetInstance();

		void Initialize(ID3D12Device* _device, ID3D12GraphicsCommandList* _commandList);

		VertexBufferId CreateVertexBuffer(const void* data, size_t dataSize, uint32_t vertexStride);
		IndexBufferId CreateIndexBuffer(const void* data, size_t dataSize, uint32_t indexStride);
		ConstantBufferId CreateConstantBuffer(const void* data, size_t dataSize);
		TextureId CreateTexture(const std::filesystem::path& fileName);
		TextureId CreateTexture(const std::vector<uint8_t>& data, const TextureInfo& textureInfo, D3D12_RESOURCE_FLAGS resourceFlags);
		BufferId CreateBuffer(const void* data, size_t dataSize, uint32_t bufferStride, uint32_t numElements, DXGI_FORMAT format);
		SamplerId CreateSampler(const D3D12_SAMPLER_DESC& samplerDesc);
		RenderTargetId CreateRenderTarget(uint64_t width, uint32_t height, DXGI_FORMAT format);
		DepthStencilId CreateDepthStencil(uint64_t width, uint32_t height, uint32_t depthBit);
		RWTextureId CreateRWTexture(const TextureInfo& textureInfo, D3D12_RESOURCE_FLAGS resourceFlags);
		RWBufferId CreateRWBuffer(const void* initialData, size_t dataSize, uint32_t bufferStride, uint32_t numElements, DXGI_FORMAT format);

		void CreateSwapChainBuffers(IDXGISwapChain4* swapChain, uint32_t buffersCount);

		const VertexBuffer& GetVertexBuffer(const VertexBufferId& resourceId) const;
		const IndexBuffer& GetIndexBuffer(const IndexBufferId& resourceId) const;
		const ConstantBuffer& GetConstantBuffer(const ConstantBufferId& resourceId) const;
		const Texture& GetTexture(const TextureId& resourceId) const;
		const Buffer& GetBuffer(const BufferId& resourceId) const;
		const Sampler& GetSampler(const SamplerId& resourceId) const;
		const RenderTarget& GetRenderTarget(const RenderTargetId& resourceId) const;
		const DepthStencil& GetDepthStencil(const DepthStencilId& resourceId) const;
		const RWTexture& GetRWTexture(const RWTextureId& resourceId) const;
		const RWBuffer& GetRWBuffer(const RWBufferId& resourceId) const;

		const D3D12_CPU_DESCRIPTOR_HANDLE& GetSwapChainDescriptorBase(uint32_t bufferId) const;
		ID3D12Resource* GetSwapChainBuffer(uint32_t bufferId) const;
		void ResetSwapChainBuffers(IDXGISwapChain4* swapChain);

		D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView(const VertexBufferId& resourceId) const;
		D3D12_INDEX_BUFFER_VIEW GetIndexBufferView(const IndexBufferId& resourceId) const;

		const D3D12_CPU_DESCRIPTOR_HANDLE& GetRenderTargetDescriptorBase(const RenderTargetId& resourceId) const;
		const D3D12_CPU_DESCRIPTOR_HANDLE& GetDepthStencilDescriptorBase(const DepthStencilId& resourceId) const;

		ID3D12DescriptorHeap* GetShaderResourceViewDescriptorHeap();

		void UpdateConstantBuffer(const ConstantBufferId& resourceId, const void* data, size_t dataSize);

		void ReleaseTemporaryUploadBuffers();

	private:
		ResourceManager() : device(nullptr), commandList(nullptr) {};
		~ResourceManager() {};

		ResourceManager(const ResourceManager&) = delete;
		ResourceManager(ResourceManager&&) = delete;
		ResourceManager& operator=(const ResourceManager&) = delete;
		ResourceManager& operator=(ResourceManager&&) = delete;

		void UploadTexture(ID3D12Resource* uploadBuffer, ID3D12Resource* targetTexture, const TextureInfo& textureInfo, const std::vector<uint8_t>& data,
			uint8_t* uploadBufferCPUAddress);
		void CopyRawDataToSubresource(const TextureInfo& srcTextureInfo, uint32_t numRows, uint16_t numSlices, uint64_t destRowPitch, uint64_t destSlicePitch,
			uint64_t rowSizeInBytes, const uint8_t* srcAddress, uint8_t* destAddress);

		ID3D12Device* device;
		ID3D12GraphicsCommandList* commandList;

		std::vector<VertexBuffer> vertexBufferPool;
		std::vector<IndexBuffer> indexBufferPool;
		std::vector<ConstantBuffer> constantBufferPool;
		std::vector<Texture> texturePool;
		std::vector<Buffer> bufferPool;
		std::vector<Sampler> samplerPool;
		std::vector<RenderTarget> renderTargetPool;
		std::vector<DepthStencil> depthStencilPool;
		std::vector<RWTexture> rwTexturePool;
		std::vector<RWBuffer> rwBufferPool;

		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> swapChainDescriptorBases;
		std::vector<ComPtr<ID3D12Resource>> swapChainBuffers;

		BufferAllocator& bufferAllocator = BufferAllocator::GetInstance();
		DescriptorAllocator& descriptorAllocator = DescriptorAllocator::GetInstance();
		TextureAllocator& textureAllocator = TextureAllocator::GetInstance();
	};
}
