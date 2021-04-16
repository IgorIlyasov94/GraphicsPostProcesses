#include "Material.h"

Graphics::Material::Material()
	: shaderList{}, firstTextureDescriptorBase{}, vertexFormat{}, renderTargetFormat{}, cullMode(D3D12_CULL_MODE_BACK), blendDesc{},
	useDepthBuffer(false), isComposed(false)
{
	SetupBlendDesc(blendDesc);
}

Graphics::Material::~Material()
{

}

void Graphics::Material::SetConstantBuffer(size_t registerIndex, void* bufferData, size_t bufferSize)
{
	constantBufferRegisterIndices.push_back(registerIndex);
	constantBufferIndices.push_back(resourceManager.CreateConstantBuffer(bufferData, bufferSize));

	constantBufferAddresses.push_back(resourceManager.GetConstantBuffer(constantBufferIndices.back()).constantBufferViewDesc.BufferLocation);
}

void Graphics::Material::SetSampler(size_t registerIndex, D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE addressU, D3D12_TEXTURE_ADDRESS_MODE addressV,
	D3D12_TEXTURE_ADDRESS_MODE addressW, uint32_t maxAnisotropy)
{
	D3D12_STATIC_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = filter;
	samplerDesc.AddressU = addressU;
	samplerDesc.AddressV = addressV;
	samplerDesc.AddressW = addressW;
	samplerDesc.MaxAnisotropy = maxAnisotropy;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.ShaderRegister = registerIndex;

	samplerDescs.push_back(samplerDesc);
}

void Graphics::Material::AssignConstantBuffer(size_t registerIndex, ConstantBufferId constantBufferId)
{
	constantBufferRegisterIndices.push_back(registerIndex);
	constantBufferIndices.push_back(constantBufferId);

	constantBufferAddresses.push_back(resourceManager.GetConstantBuffer(constantBufferId).constantBufferViewDesc.BufferLocation);
}

void Graphics::Material::AssignTexture(ID3D12GraphicsCommandList* commandList, size_t registerIndex, TextureId textureId, bool asPixelShaderResource)
{
	textureRegisterIndices.push_back(registerIndex);
	textureIndices.push_back(textureId);

	SetResourceBarrier(commandList, resourceManager.GetTexture(textureId).textureAllocation.textureResource, D3D12_RESOURCE_STATE_COMMON,
		(asPixelShaderResource) ? D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE : D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

void Graphics::Material::SetVertexShader(D3D12_SHADER_BYTECODE shaderBytecode)
{
	shaderList.vertexShader = shaderBytecode;
}

void Graphics::Material::SetHullShader(D3D12_SHADER_BYTECODE shaderBytecode)
{
	shaderList.hullShader = shaderBytecode;
}

void Graphics::Material::SetDomainShader(D3D12_SHADER_BYTECODE shaderBytecode)
{
	shaderList.domainShader = shaderBytecode;
}

void Graphics::Material::SetGeometryShader(D3D12_SHADER_BYTECODE shaderBytecode)
{
	shaderList.geometryShader = shaderBytecode;
}

void Graphics::Material::SetPixelShader(D3D12_SHADER_BYTECODE shaderBytecode)
{
	shaderList.pixelShader = shaderBytecode;
}

void Graphics::Material::SetVertexFormat(VertexFormat format)
{
	vertexFormat = format;
}

void Graphics::Material::SetRenderTargetFormat(size_t renderTargetIndex, DXGI_FORMAT format)
{
	if (renderTargetIndex > 7)
		throw std::exception("Material::SetRenderTargetFormat: Render target index must be from 0 to 7");

	renderTargetFormat[renderTargetIndex] = format;
}

void Graphics::Material::SetBlendMode(bool blendOn, D3D12_BLEND srcBlend, D3D12_BLEND destBlend, D3D12_BLEND_OP blendOp, D3D12_BLEND srcBlendAlpha,
	D3D12_BLEND destBlendAlpha, D3D12_BLEND_OP blendOpAlpha)
{
	SetupBlendDesc(blendDesc, blendOn, srcBlend, destBlend, blendOp, srcBlendAlpha, destBlendAlpha, blendOpAlpha);
}

void Graphics::Material::UpdateConstantBuffer(size_t registerIndex, void* bufferData, size_t bufferSize)
{

}

void Graphics::Material::Compose(ID3D12Device* device)
{
	if (isComposed)
	{
		rootSignature.Reset();
		pipelineState.Reset();

		isComposed = false;
	}

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs;
	CreateInputElementDescs(vertexFormat, inputElementDescs);

	D3D12_RASTERIZER_DESC rasterizerDesc;
	SetupRasterizerDesc(rasterizerDesc, cullMode);

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	SetupDepthStencilDesc(depthStencilDesc, useDepthBuffer);

	std::vector<size_t> textureDescriptorIndices;

	for (auto& textureId: textureIndices)
		textureDescriptorIndices.push_back(resourceManager.GetTexture(textureId).descriptorAllocation.descriptorStartIndex);

	std::vector<D3D12_DESCRIPTOR_RANGE> textureDescRange;
	D3D12_ROOT_DESCRIPTOR_TABLE textureRootDescTable;

	CreateTextureRootDescriptorTable(textureRegisterIndices, textureDescriptorIndices, textureDescRange, textureRootDescTable);
	CreatePipelineStateAndRootSignature(device, { inputElementDescs.data() , inputElementDescs.size() }, rasterizerDesc, blendDesc, depthStencilDesc,
		renderTargetFormat, shaderList, constantBufferRegisterIndices, textureRootDescTable, samplerDescs, &rootSignature, &pipelineState);

	if (textureIndices.size() > 0)
		firstTextureDescriptorBase = resourceManager.GetTexture(textureIndices.front()).descriptorAllocation.gpuDescriptorBase;

	isComposed = true;
}

bool Graphics::Material::IsComposed() const noexcept
{
	return isComposed;
}

void Graphics::Material::Present(ID3D12GraphicsCommandList* commandList) const
{
	commandList->SetPipelineState(pipelineState.Get());
	commandList->SetGraphicsRootSignature(rootSignature.Get());

	uint32_t rootParameterIndex{};

	for (auto& constantBufferAddress: constantBufferAddresses)
		commandList->SetGraphicsRootConstantBufferView(rootParameterIndex++, constantBufferAddress);

	if (textureIndices.size() > 0)
		commandList->SetGraphicsRootDescriptorTable(rootParameterIndex++, firstTextureDescriptorBase);
}

void Graphics::Material::CreateInputElementDescs(VertexFormat format, std::vector<D3D12_INPUT_ELEMENT_DESC>& inputElementDescs) const noexcept
{
	inputElementDescs.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

	if (format == VertexFormat::POSITION_NORMAL || format == VertexFormat::POSITION_NORMAL_TEXCOORD)
		inputElementDescs.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

	if (format == VertexFormat::POSITION_TEXCOORD || format == VertexFormat::POSITION_NORMAL_TEXCOORD)
		inputElementDescs.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
}
