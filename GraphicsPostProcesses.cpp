#include "GraphicsPostProcesses.h"
#include "Resources/Shaders/ScreenQuad.vsh.h"
#include "Resources/Shaders/HDRToneMapping.psh.h"

GraphicsPostProcesses::GraphicsPostProcesses()
	: sceneViewport{}, renderTargetViewDescriptorSize(0),// shaderResourceViewDescriptorSize(0),
	hdrConstantBuffer{}
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

	//renderTargetViewDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//shaderResourceViewDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//constantBufferSizes.push_back(sizeof(HdrConstantBuffer));
	//constantBufferSizes.push_back(sizeof(HdrConstantBuffer));

	//CreateDescriptorHeap(device, constantBufferSizes.size(), D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
	//	&shaderResourceViewDescHeap);
	
	hdrConstantBuffer.shiftVector = { 1.0f, 1.0f, 1.0f };
	hdrConstantBuffer.middleGray = 0.6f;
	hdrConstantBuffer.whiteCutoff = 0.8f;
	hdrConstantBuffer.brightPassOffset = 5.0f;
	hdrConstantBuffer.brightPassThreshold = 10.0f;

	//constantBuffersData.push_back(reinterpret_cast<uint8_t*>(&hdrConstantBuffer));
	//constantBuffersData.push_back(reinterpret_cast<uint8_t*>(&hdrConstantBuffer));

	//CreateConstantBuffer(device, constantBuffersData, constantBufferSizes, shaderResourceViewDescHeap.Get(), &constantBuffer, commandList);

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

	//CreateVertexBuffer(device, reinterpret_cast<uint8_t*>(vertices), sizeof(vertices), sizeof(ScreenQuadVertex),
	//	screenQuadVertexBufferView, &screenQuadVertexBuffer, &screenQuadVertexBufferUpload, commandList);

	renderTargetViewDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}

void GraphicsPostProcesses::EnableHDR(ID3D12GraphicsCommandList* commandList, ID3D12DescriptorHeap* outputRenderTargetDescHeap, size_t bufferIndex)
{
	commandList->SetPipelineState(hdrPipelineState.Get());

	commandList->SetGraphicsRootSignature(hdrRootSignature.Get());
	
	//ID3D12DescriptorHeap* shaderResourceDescriptorHeaps[] = { shaderResourceViewDescHeap.Get() };

	//commandList->SetDescriptorHeaps(_countof(shaderResourceDescriptorHeaps), shaderResourceDescriptorHeaps);

	//D3D12_GPU_DESCRIPTOR_HANDLE shaderResurceViewHandle(shaderResourceViewDescHeap->GetGPUDescriptorHandleForHeapStart());
	
	//commandList->SetGraphicsRootDescriptorTable(0, shaderResurceViewHandle);
	//commandList->SetGraphicsRootConstantBufferView(0, constantBuffer->GetGPUVirtualAddress());
	
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	commandList->IASetVertexBuffers(0, 1, &resourceManager.GetVertexBuffer(screenQuadVertexBufferId).vertexBufferView);
	commandList->RSSetViewports(1, &sceneViewport);
	commandList->RSSetScissorRects(1, &sceneScissorRect);
	
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetHandle(outputRenderTargetDescHeap->GetCPUDescriptorHandleForHeapStart());
	renderTargetHandle.ptr += bufferIndex * renderTargetViewDescriptorSize;

	commandList->OMSetRenderTargets(1, &renderTargetHandle, false, nullptr);
	
	commandList->DrawInstanced(4, 1, 0, 0);
}