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

void setupRasterizerDesc(D3D12_RASTERIZER_DESC& rasterizerDesc, D3D12_CULL_MODE cullMode = D3D12_CULL_MODE_NONE)
{
	rasterizerDesc.AntialiasedLineEnable = false;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	rasterizerDesc.CullMode = cullMode;
	rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
}

void setupBlendDesc(D3D12_BLEND_DESC& blendDesc, bool blendOn = false, 
	D3D12_BLEND srcBlend = D3D12_BLEND_ONE, D3D12_BLEND destBlend = D3D12_BLEND_ZERO, D3D12_BLEND_OP blendOp = D3D12_BLEND_OP_ADD,
	D3D12_BLEND srcBlendAlpha = D3D12_BLEND_ONE, D3D12_BLEND destBlendAlpha = D3D12_BLEND_ZERO, D3D12_BLEND_OP blendOpAlpha = D3D12_BLEND_OP_ADD)
{
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	
	const D3D12_RENDER_TARGET_BLEND_DESC rtBlendDescDefault =
	{
		false, false,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP, D3D12_COLOR_WRITE_ENABLE_ALL
	};

	for (auto renderTargetId = 0; renderTargetId < 8; renderTargetId++)
	{
		blendDesc.RenderTarget[renderTargetId] = rtBlendDescDefault;
	}

	blendDesc.RenderTarget[0].BlendEnable = blendOn;
	blendDesc.RenderTarget[0].SrcBlend = srcBlend;
	blendDesc.RenderTarget[0].DestBlend = destBlend;
	blendDesc.RenderTarget[0].BlendOp = blendOp;
	blendDesc.RenderTarget[0].SrcBlendAlpha = srcBlendAlpha;
	blendDesc.RenderTarget[0].DestBlendAlpha = destBlendAlpha;
	blendDesc.RenderTarget[0].BlendOpAlpha = blendOpAlpha;
}

void setupDepthStencilDesc(D3D12_DEPTH_STENCIL_DESC& depthStencilDesc, bool depthEnable)
{
	depthStencilDesc.DepthEnable = depthEnable;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.StencilEnable = false;
	depthStencilDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	depthStencilDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	
	const D3D12_DEPTH_STENCILOP_DESC depthStencilOpDesc =
	{
		D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS
	};

	depthStencilDesc.BackFace = depthStencilOpDesc;
	depthStencilDesc.FrontFace = depthStencilOpDesc;
}