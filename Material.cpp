#include "Material.h"

Graphics::Material::Material()
	: shaderList{}, vertexFormat{}, primitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE), renderTargetFormat{}, depthStencilFormat{},
	cullMode(D3D12_CULL_MODE_NONE), blendDesc{}, useDepthBuffer(false), isComposed(false)
{
	SetupBlendDesc(blendDesc);
}

Graphics::Material::~Material()
{

}

Graphics::ConstantBufferId Graphics::Material::SetConstantBuffer(size_t registerIndex, void* bufferData, size_t bufferSize)
{
	registerSet.constantBufferRegisterIndices.push_back(registerIndex);
	indexSet.constantBufferIndices.push_back(resourceManager.CreateConstantBuffer(bufferData, bufferSize));

	constantBufferAddresses.push_back(resourceManager.GetConstantBuffer(indexSet.constantBufferIndices.back()).constantBufferViewDesc.BufferLocation);

	return indexSet.constantBufferIndices.back();
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
	samplerDesc.ShaderRegister = static_cast<uint32_t>(registerIndex);

	samplerDescs.push_back(samplerDesc);
}

void Graphics::Material::AssignConstantBuffer(size_t registerIndex, ConstantBufferId constantBufferId)
{
	registerSet.constantBufferRegisterIndices.push_back(registerIndex);
	indexSet.constantBufferIndices.push_back(constantBufferId);

	constantBufferAddresses.push_back(resourceManager.GetConstantBuffer(constantBufferId).constantBufferViewDesc.BufferLocation);
}

void Graphics::Material::AssignTexture(ID3D12GraphicsCommandList* commandList, size_t registerIndex, TextureId textureId, bool asPixelShaderResource)
{
	registerSet.textureRegisterIndices.push_back(registerIndex);
	indexSet.textureIndices.push_back(textureId);

	resourceManager.SetResourceBarrier(commandList, textureId, D3D12_RESOURCE_BARRIER_FLAG_NONE,
		(asPixelShaderResource) ? D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE : D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

void Graphics::Material::AssignTexture(size_t registerIndex, RWTextureId rwTextureId)
{
	registerSet.rwTextureReadOnlyRegisterIndices.push_back(registerIndex);
	indexSet.rwTextureReadOnlyIndices.push_back(rwTextureId);
}

void Graphics::Material::AssignRenderTexture(size_t registerIndex, RenderTargetId renderTargetId)
{
	registerSet.renderTextureRegisterIndices.push_back(registerIndex);
	indexSet.renderTargetIndices.push_back(renderTargetId);
}

void Graphics::Material::AssignDepthTexture(size_t registerIndex, DepthStencilId depthStencilId)
{
	registerSet.depthTextureRegisterIndices.push_back(registerIndex);
	indexSet.depthStencilIndices.push_back(depthStencilId);
}

void Graphics::Material::AssignBuffer(size_t registerIndex, BufferId bufferId)
{
	registerSet.bufferRegisterIndices.push_back(registerIndex);
	indexSet.bufferIndices.push_back(bufferId);
}

void Graphics::Material::AssignBuffer(size_t registerIndex, RWBufferId rwBufferId)
{
	registerSet.rwBufferReadOnlyRegisterIndices.push_back(registerIndex);
	indexSet.rwBufferReadOnlyIndices.push_back(rwBufferId);
}

void Graphics::Material::AssignRWTexture(size_t registerIndex, RWTextureId rwTextureId)
{
	registerSet.rwTextureRegisterIndices.push_back(registerIndex);
	indexSet.rwTextureIndices.push_back(rwTextureId);
}

void Graphics::Material::AssignRWBuffer(size_t registerIndex, RWBufferId rwBufferId)
{
	registerSet.rwBufferRegisterIndices.push_back(registerIndex);
	indexSet.rwBufferIndices.push_back(rwBufferId);
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

void Graphics::Material::AddCustomInputLayoutElement(std::string semanticName, size_t semanticIndex, DXGI_FORMAT format, D3D12_INPUT_CLASSIFICATION inputClassification,
	uint32_t instanceDataStepRate)
{
	semanticNames.push_back(std::shared_ptr<std::string>(new std::string(semanticName)));

	D3D12_INPUT_ELEMENT_DESC inputElementDesc{};
	inputElementDesc.SemanticName = semanticNames.back()->data();
	inputElementDesc.SemanticIndex = static_cast<uint32_t>(semanticIndex);
	inputElementDesc.Format = format;
	inputElementDesc.InputSlot = 0;
	inputElementDesc.AlignedByteOffset = (inputElementDescs.empty()) ? 0 : D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDesc.InputSlotClass = inputClassification;
	inputElementDesc.InstanceDataStepRate = instanceDataStepRate;

	inputElementDescs.push_back(inputElementDesc);
}

void Graphics::Material::SetVertexFormat(VertexFormat format)
{
	vertexFormat = format;
}

void Graphics::Material::SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE type)
{
	primitiveTopologyType = type;
}

void Graphics::Material::SetRenderTargetFormat(size_t renderTargetIndex, DXGI_FORMAT format)
{
	if (renderTargetIndex > 7)
		throw std::exception("Material::SetRenderTargetFormat: Render target index must be from 0 to 7");

	renderTargetFormat[renderTargetIndex] = format;
}

void Graphics::Material::SetDepthStencilFormat(uint32_t depthBit)
{
	depthStencilFormat = (depthBit == 32) ? DXGI_FORMAT_D32_FLOAT : DXGI_FORMAT_D24_UNORM_S8_UINT;
}

void Graphics::Material::SetDepthTest(bool _useDepthBuffer)
{
	useDepthBuffer = _useDepthBuffer;
}

void Graphics::Material::SetCullMode(D3D12_CULL_MODE _cullMode)
{
	cullMode = _cullMode;
}

void Graphics::Material::SetBlendMode(bool blendOn, D3D12_BLEND srcBlend, D3D12_BLEND destBlend, D3D12_BLEND_OP blendOp, D3D12_BLEND srcBlendAlpha,
	D3D12_BLEND destBlendAlpha, D3D12_BLEND_OP blendOpAlpha)
{
	SetupBlendDesc(blendDesc, blendOn, srcBlend, destBlend, blendOp, srcBlendAlpha, destBlendAlpha, blendOpAlpha);
}

void Graphics::Material::UpdateConstantBuffer(ConstantBufferId constantBufferId, void* bufferData, size_t bufferSize)
{
	resourceManager.UpdateConstantBuffer(constantBufferId, bufferData, bufferSize);
}

void Graphics::Material::Compose(ID3D12Device* device)
{
	if (isComposed)
	{
		rootSignature.Reset();
		pipelineState.Reset();

		isComposed = false;
	}

	if (inputElementDescs.empty())
		CreateInputElementDescs(vertexFormat, inputElementDescs);

	D3D12_RASTERIZER_DESC rasterizerDesc;
	SetupRasterizerDesc(rasterizerDesc, cullMode);

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	SetupDepthStencilDesc(depthStencilDesc, useDepthBuffer);

	std::vector<D3D12_DESCRIPTOR_RANGE> textureDescRanges;
	std::vector<D3D12_ROOT_DESCRIPTOR_TABLE> textureRootDescTables;

	CreateResourceRootDescriptorTables(registerSet, textureDescRanges, textureRootDescTables);
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	std::vector<D3D12_ROOT_PARAMETER> rootParameters;
	CreateRootParameters(shaderList, registerSet, textureRootDescTables, rootSignatureFlags, rootParameters);

	CreateRootSignature(device, rootParameters, samplerDescs, rootSignatureFlags, &rootSignature);

	CreateGraphicsPipelineState(device, { inputElementDescs.data() , static_cast<uint32_t>(inputElementDescs.size()) }, rootSignature.Get(), rasterizerDesc,
		blendDesc, depthStencilDesc, renderTargetFormat, depthStencilFormat, shaderList, &pipelineState);

	for (auto& textureIndex : indexSet.textureIndices)
		firstResourceDescriptorBases.push_back(resourceManager.GetTexture(textureIndex).shaderResourceDescriptorAllocation.gpuDescriptorBase);

	for (auto& rwTextureIndex : indexSet.rwTextureReadOnlyIndices)
		firstResourceDescriptorBases.push_back(resourceManager.GetRWTexture(rwTextureIndex).shaderResourceDescriptorAllocation.gpuDescriptorBase);

	for (auto& renderTargetIndex : indexSet.renderTargetIndices)
		firstResourceDescriptorBases.push_back(resourceManager.GetRenderTarget(renderTargetIndex).shaderResourceDescriptorAllocation.gpuDescriptorBase);

	for (auto& depthStencilIndex : indexSet.depthStencilIndices)
		firstResourceDescriptorBases.push_back(resourceManager.GetDepthStencil(depthStencilIndex).shaderResourceDescriptorAllocation.gpuDescriptorBase);

	for (auto& bufferIndex : indexSet.bufferIndices)
		firstResourceDescriptorBases.push_back(resourceManager.GetBuffer(bufferIndex).shaderResourceDescriptorAllocation.gpuDescriptorBase);

	for (auto& rwBufferIndex : indexSet.rwBufferReadOnlyIndices)
		firstResourceDescriptorBases.push_back(resourceManager.GetRWBuffer(rwBufferIndex).shaderResourceDescriptorAllocation.gpuDescriptorBase);

	for (auto& rwTextureIndex : indexSet.rwTextureIndices)
		firstResourceDescriptorBases.push_back(resourceManager.GetRWTexture(rwTextureIndex).unorderedAccessDescriptorAllocation.gpuDescriptorBase);

	for (auto& rwBufferIndex : indexSet.rwBufferIndices)
		firstResourceDescriptorBases.push_back(resourceManager.GetRWBuffer(rwBufferIndex).unorderedAccessDescriptorAllocation.gpuDescriptorBase);

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

	for (auto& firstTextureDescriptorBase : firstResourceDescriptorBases)
		commandList->SetGraphicsRootDescriptorTable(rootParameterIndex++, firstTextureDescriptorBase);
}

void Graphics::Material::CreateInputElementDescs(VertexFormat format, std::vector<D3D12_INPUT_ELEMENT_DESC>& inputElementDescs) const noexcept
{
	if (format != VertexFormat::UNDEFINED)
		inputElementDescs.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

	if ((format & VertexFormat::NORMAL) == VertexFormat::NORMAL)
		inputElementDescs.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

	if ((format & VertexFormat::TANGENT_BINORMAL) == VertexFormat::TANGENT_BINORMAL)
	{
		inputElementDescs.push_back({ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		inputElementDescs.push_back({ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	}

	if ((format & VertexFormat::TEXCOORD) == VertexFormat::TEXCOORD)
		inputElementDescs.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
}

void Graphics::Material::CreateResourceRootDescriptorTables(const RegisterSet& _registerSet, std::vector<D3D12_DESCRIPTOR_RANGE>& descriptorRanges,
	std::vector<D3D12_ROOT_DESCRIPTOR_TABLE>& rootDescriptorTables)
{
	descriptorRanges.clear();

	D3D12_DESCRIPTOR_RANGE descRange{};
	descRange.NumDescriptors = 1;
	descRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

	for (auto& textureRegisterId : _registerSet.textureRegisterIndices)
	{
		descRange.BaseShaderRegister = static_cast<uint32_t>(textureRegisterId);
		descriptorRanges.push_back(descRange);
	}

	for (auto& rwTextureRegisterId : _registerSet.rwTextureReadOnlyRegisterIndices)
	{
		descRange.BaseShaderRegister = static_cast<uint32_t>(rwTextureRegisterId);
		descriptorRanges.push_back(descRange);
	}

	for (auto& renderTextureRegisterId : _registerSet.renderTextureRegisterIndices)
	{
		descRange.BaseShaderRegister = static_cast<uint32_t>(renderTextureRegisterId);
		descriptorRanges.push_back(descRange);
	}

	for (auto& depthTextureRegisterId : _registerSet.depthTextureRegisterIndices)
	{
		descRange.BaseShaderRegister = static_cast<uint32_t>(depthTextureRegisterId);
		descriptorRanges.push_back(descRange);
	}

	for (auto& bufferRegisterId : _registerSet.bufferRegisterIndices)
	{
		descRange.BaseShaderRegister = static_cast<uint32_t>(bufferRegisterId);
		descriptorRanges.push_back(descRange);
	}

	for (auto& rwBufferRegisterId : _registerSet.rwBufferReadOnlyRegisterIndices)
	{
		descRange.BaseShaderRegister = static_cast<uint32_t>(rwBufferRegisterId);
		descriptorRanges.push_back(descRange);
	}

	descRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;

	for (auto& rwTextureRegisterId : _registerSet.rwTextureRegisterIndices)
	{
		descRange.BaseShaderRegister = static_cast<uint32_t>(rwTextureRegisterId);
		descriptorRanges.push_back(descRange);
	}

	for (auto& rwBufferRegisterId : _registerSet.rwBufferRegisterIndices)
	{
		descRange.BaseShaderRegister = static_cast<uint32_t>(rwBufferRegisterId);
		descriptorRanges.push_back(descRange);
	}

	for (auto& descriptorRange : descriptorRanges)
	{
		D3D12_ROOT_DESCRIPTOR_TABLE rootDescriptorTable{};
		rootDescriptorTable.NumDescriptorRanges = 1;
		rootDescriptorTable.pDescriptorRanges = &descriptorRange;

		rootDescriptorTables.push_back(rootDescriptorTable);
	}
}

void Graphics::Material::CreateRootParameters(const ShaderList& shaderList, const RegisterSet& _registerSet,
	const std::vector<D3D12_ROOT_DESCRIPTOR_TABLE>& rootDescriptorTables, D3D12_ROOT_SIGNATURE_FLAGS& rootSignatureFlags, std::vector<D3D12_ROOT_PARAMETER>& rootParameters)
{
	if (shaderList.vertexShader.pShaderBytecode == nullptr)
		rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;

	if (shaderList.hullShader.pShaderBytecode == nullptr)
		rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

	if (shaderList.domainShader.pShaderBytecode == nullptr)
		rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;

	if (shaderList.geometryShader.pShaderBytecode == nullptr)
		rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	if (shaderList.pixelShader.pShaderBytecode == nullptr)
		rootSignatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	for (auto& constantBufferIndex : _registerSet.constantBufferRegisterIndices)
	{
		D3D12_ROOT_PARAMETER rootParameter{};

		rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParameter.Descriptor.RegisterSpace = 0;
		rootParameter.Descriptor.ShaderRegister = static_cast<uint32_t>(constantBufferIndex);
		rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		rootParameters.push_back(rootParameter);
	}

	for (auto& rootDescriptorTable : rootDescriptorTables)
	{
		D3D12_ROOT_PARAMETER rootParameter{};
		rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParameter.DescriptorTable = rootDescriptorTable;
		rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		
		rootParameters.push_back(rootParameter);
	}
}

void Graphics::Material::CreateRootSignature(ID3D12Device* device, const std::vector<D3D12_ROOT_PARAMETER>& rootParameters,
	const std::vector<D3D12_STATIC_SAMPLER_DESC>& samplerDescs, D3D12_ROOT_SIGNATURE_FLAGS flags, ID3D12RootSignature** rootSignature)
{
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.NumParameters = static_cast<uint32_t>(rootParameters.size());

	if (rootSignatureDesc.NumParameters > 0)
		rootSignatureDesc.pParameters = rootParameters.data();

	rootSignatureDesc.NumStaticSamplers = static_cast<uint32_t>(samplerDescs.size());

	if (rootSignatureDesc.NumStaticSamplers > 0)
		rootSignatureDesc.pStaticSamplers = samplerDescs.data();

	rootSignatureDesc.Flags = flags;

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;

	if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signature, &error)))
	{
		std::stringstream errorMessage;
		errorMessage << "Material::CreateRootSignature: Root Signature serialization failed!\n";
		errorMessage << reinterpret_cast<char*>(error->GetBufferPointer());

		throw std::runtime_error(errorMessage.str());
	}

	ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(rootSignature)),
		"Material::CreateRootSignature: Root Signature creating failed!");
}

void Graphics::Material::CreateGraphicsPipelineState(ID3D12Device* device, const D3D12_INPUT_LAYOUT_DESC& inputLayoutDesc, ID3D12RootSignature* rootSignature,
	const D3D12_RASTERIZER_DESC& rasterizerDesc, const D3D12_BLEND_DESC& blendDesc, const D3D12_DEPTH_STENCIL_DESC& depthStencilDesc,
	const std::array<DXGI_FORMAT, 8>& rtvFormat, DXGI_FORMAT dsvFormat, const ShaderList& shaderList, ID3D12PipelineState** pipelineState)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{};
	pipelineStateDesc.InputLayout = inputLayoutDesc;
	pipelineStateDesc.pRootSignature = rootSignature;
	pipelineStateDesc.RasterizerState = rasterizerDesc;
	pipelineStateDesc.BlendState = blendDesc;
	pipelineStateDesc.VS = shaderList.vertexShader;
	pipelineStateDesc.HS = shaderList.hullShader;
	pipelineStateDesc.DS = shaderList.domainShader;
	pipelineStateDesc.GS = shaderList.geometryShader;
	pipelineStateDesc.PS = shaderList.pixelShader;
	pipelineStateDesc.DepthStencilState = depthStencilDesc;
	pipelineStateDesc.DSVFormat = dsvFormat;
	pipelineStateDesc.SampleMask = UINT_MAX;
	pipelineStateDesc.PrimitiveTopologyType = primitiveTopologyType;
	pipelineStateDesc.NumRenderTargets = 0;

	for (uint32_t formatId = 0; formatId < rtvFormat.size(); formatId++)
	{
		pipelineStateDesc.RTVFormats[formatId] = rtvFormat[formatId];

		if (rtvFormat[formatId] != DXGI_FORMAT_UNKNOWN)
			pipelineStateDesc.NumRenderTargets++;
	}

	pipelineStateDesc.SampleDesc.Count = 1;

	ThrowIfFailed(device->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(pipelineState)),
		"Material::CreateGraphicsPipelineState: Graphics Pipeline State creating failed!");
}
