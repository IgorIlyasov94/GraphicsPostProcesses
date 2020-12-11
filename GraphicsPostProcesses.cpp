#include "GraphicsPostProcesses.h"

GraphicsPostProcesses::GraphicsPostProcesses()
{

}

GraphicsPostProcesses& GraphicsPostProcesses::GetInstance()
{
	static GraphicsPostProcesses instance;

	return instance;
}

void GraphicsPostProcesses::Initialize(const int32_t& resolutionX, const int32_t& resolutionY, ID3D12Device* device)
{
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = sizeof(inputElementDescs);

	CreateRootSignature(device, hdrRootSignature.Get());

	D3D12_RASTERIZER_DESC rasterizerDesc;
	SetupRasterizerDesc(rasterizerDesc, D3D12_CULL_MODE_BACK);

	D3D12_BLEND_DESC blendDesc;
	SetupBlendDesc(blendDesc);

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	SetupDepthStencilDesc(depthStencilDesc, true);

	ComPtr<ID3DBlob> quadVertexShader;
	CreateVertexShader(L"Resources\\Shaders\\ScreenQuad.vsh", quadVertexShader.Get());

	ComPtr<ID3DBlob> toneMappingPixelShader;
	CreateVertexShader(L"Resources\\Shaders\\HDRToneMapping.psh", toneMappingPixelShader.Get());

	CreateGraphicsPipelineState(device, inputLayoutDesc, hdrRootSignature.Get(), rasterizerDesc, blendDesc, depthStencilDesc,
		DXGI_FORMAT_R8G8B8A8_UNORM, quadVertexShader.Get(), toneMappingPixelShader.Get(), hdrPipelineState.Get());
}

void GraphicsPostProcesses::EnableHDR()
{

}