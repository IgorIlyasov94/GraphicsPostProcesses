#include "GraphicsHelper.h"

void createRootSignature(ID3D12Device*& device, ID3D12RootSignature*& rootSignature)
{
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;

	ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
	ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
}

void createVertexShader(std::wstring shaderPath, ID3DBlob*& vertexShader)
{
	vertexShader = nullptr;

#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	ThrowIfFailed(D3DCompileFromFile(shaderPath.c_str(), nullptr, nullptr, "main", "vs_6_0", compileFlags, 0, &vertexShader, nullptr));
}

void createPixelShader(std::wstring shaderPath, ID3DBlob*& pixelShader)
{
	pixelShader = nullptr;

#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	ThrowIfFailed(D3DCompileFromFile(shaderPath.c_str(), nullptr, nullptr, "main", "ps_6_0", compileFlags, 0, &pixelShader, nullptr));
}

void createGraphicsPipelineState(ID3D12Device*& device, D3D12_INPUT_LAYOUT_DESC& inputLayoutDesc, ID3D12RootSignature*& rootSignature,
	D3D12_RASTERIZER_DESC& rasterizerDesc, D3D12_BLEND_DESC& blendDesc, D3D12_DEPTH_STENCIL_DESC& depthStencilDesc,
	DXGI_FORMAT rtvFormat, ID3DBlob*& vertexShader, ID3DBlob*& pixelShader, ID3D12PipelineState*& pipelineState)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{};
	pipelineStateDesc.InputLayout = inputLayoutDesc;
	pipelineStateDesc.pRootSignature = rootSignature;
	pipelineStateDesc.RasterizerState = rasterizerDesc;
	pipelineStateDesc.BlendState = blendDesc;
	pipelineStateDesc.VS = { reinterpret_cast<uint8_t*>(vertexShader->GetBufferPointer()), vertexShader->GetBufferSize() };
	pipelineStateDesc.PS = { reinterpret_cast<uint8_t*>(pixelShader->GetBufferPointer()), pixelShader->GetBufferSize() };
	pipelineStateDesc.DepthStencilState = depthStencilDesc;
	pipelineStateDesc.SampleMask = UINT_MAX;
	pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateDesc.NumRenderTargets = 1;
	pipelineStateDesc.RTVFormats[0] = rtvFormat;
	pipelineStateDesc.SampleDesc.Count = 1;

	ThrowIfFailed(device->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(&pipelineState)));
}