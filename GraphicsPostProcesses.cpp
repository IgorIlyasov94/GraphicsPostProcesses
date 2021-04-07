#include "GraphicsPostProcesses.h"
#include "Resources/Shaders/ScreenQuad.vsh.h"
#include "Resources/Shaders/HDRToneMapping.psh.h"

GraphicsPostProcesses::GraphicsPostProcesses()
	: sceneViewport{}, renderTargetViewDescriptorSize(0), hdrConstantBuffer{}
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

	D3D12_RASTERIZER_DESC rasterizerDesc;
	SetupRasterizerDesc(rasterizerDesc, D3D12_CULL_MODE_NONE);

	D3D12_BLEND_DESC blendDesc;
	SetupBlendDesc(blendDesc);

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	SetupDepthStencilDesc(depthStencilDesc, false);

	ShaderList hdrShaderList{};
	hdrShaderList.vertexShader = { quadVertexShader, sizeof(quadVertexShader) };
	hdrShaderList.pixelShader = { toneMappingPixelShader, sizeof(toneMappingPixelShader) };

	noiseTextureId = std::move(resourceManager.CreateTexture("Resources\\Textures\\Noise.dds"));

	const std::set<size_t> hdrConstantBufferIndices = { 0 };
	const std::vector<size_t> hdrTextureIndices = { 0 };
	const std::vector<size_t> hdrTextureDescriptorIndices = { resourceManager.GetTexture(noiseTextureId).descriptorAllocation.descriptorStartIndex };
	const std::vector<D3D12_STATIC_SAMPLER_DESC> postProcessSamplerDescs;

	std::vector<D3D12_DESCRIPTOR_RANGE> hdrTextureDescRange;
	D3D12_ROOT_DESCRIPTOR_TABLE hdrTextureRootDescTable;

	CreateTextureRootDescriptorTable(hdrTextureIndices, hdrTextureDescriptorIndices, hdrTextureDescRange, hdrTextureRootDescTable);
	CreatePipelineStateAndRootSignature(device, { inputElementDescs , _countof(inputElementDescs) }, rasterizerDesc, blendDesc, depthStencilDesc,
		DXGI_FORMAT_R8G8B8A8_UNORM, hdrShaderList, hdrConstantBufferIndices, hdrTextureRootDescTable, postProcessSamplerDescs, &hdrRootSignature, &hdrPipelineState);

	hdrConstantBuffer.shiftVector = { 0.8f, 0.6f, 0.7f };
	hdrConstantBuffer.middleGray = 0.6f;
	hdrConstantBuffer.whiteCutoff = 0.8f;
	hdrConstantBuffer.brightPassOffset = 5.0f;
	hdrConstantBuffer.brightPassThreshold = 10.0f;

	ScreenQuadVertex vertices[] =
	{
		{ XMFLOAT3(-1.0f, 1.0f, 0.5f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 0.5f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, 0.5f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 0.5f), XMFLOAT2(1.0f, 1.0f) }
	};

	std::vector<uint8_t> vertexBufferRawData;

	std::copy(reinterpret_cast<uint8_t*>(&vertices[0]), reinterpret_cast<uint8_t*>(&vertices[0]) + sizeof(vertices), std::back_inserter(vertexBufferRawData));

	screenQuadVertexBufferId = std::move(resourceManager.CreateVertexBuffer(vertexBufferRawData, sizeof(ScreenQuadVertex)));

	std::vector<uint8_t> hdrConstantBufferRawData;

	std::copy(reinterpret_cast<uint8_t*>(&hdrConstantBuffer), reinterpret_cast<uint8_t*>(&hdrConstantBuffer) + sizeof(HdrConstantBuffer), std::back_inserter(hdrConstantBufferRawData));

	hdrConstantBufferId = std::move(resourceManager.CreateConstantBuffer(hdrConstantBufferRawData));

	renderTargetViewDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	
	SetResourceBarrier(commandList, resourceManager.GetTexture(noiseTextureId).textureAllocation.textureResource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void GraphicsPostProcesses::EnableHDR(ID3D12GraphicsCommandList* commandList, ID3D12DescriptorHeap* outputRenderTargetDescHeap, size_t bufferIndex)
{
	commandList->SetPipelineState(hdrPipelineState.Get());

	commandList->SetGraphicsRootSignature(hdrRootSignature.Get());
	
	ID3D12DescriptorHeap* descHeaps[] = { resourceManager.GetTexture(noiseTextureId).descriptorAllocation.descriptorHeap };

	commandList->SetDescriptorHeaps(_countof(descHeaps), descHeaps);

	commandList->SetGraphicsRootConstantBufferView(0, resourceManager.GetConstantBuffer(hdrConstantBufferId).constantBufferViewDesc.BufferLocation);
	commandList->SetGraphicsRootDescriptorTable(1, resourceManager.GetTexture(noiseTextureId).descriptorAllocation.gpuDescriptorBase);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	commandList->IASetVertexBuffers(0, 1, &resourceManager.GetVertexBuffer(screenQuadVertexBufferId).vertexBufferView);
	commandList->RSSetViewports(1, &sceneViewport);
	commandList->RSSetScissorRects(1, &sceneScissorRect);
	
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetHandle(outputRenderTargetDescHeap->GetCPUDescriptorHandleForHeapStart());
	renderTargetHandle.ptr += bufferIndex * renderTargetViewDescriptorSize;

	commandList->OMSetRenderTargets(1, &renderTargetHandle, false, nullptr);
	
	commandList->DrawInstanced(4, 1, 0, 0);
}