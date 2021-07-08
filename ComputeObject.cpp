#include "ComputeObject.h"

Graphics::ComputeObject::ComputeObject()
	: computeShader{}, threadGroupCountX(1), threadGroupCountY(1), threadGroupCountZ(1), isComposed(false)
{

}

Graphics::ComputeObject::~ComputeObject()
{

}

Graphics::ConstantBufferId Graphics::ComputeObject::SetConstantBuffer(size_t registerIndex, void* bufferData, size_t bufferSize)
{
	registerSet.constantBufferRegisterIndices.push_back(registerIndex);
	indexSet.constantBufferIndices.push_back(resourceManager.CreateConstantBuffer(bufferData, bufferSize));

	constantBufferAddresses.push_back(resourceManager.GetConstantBuffer(indexSet.constantBufferIndices.back()).constantBufferViewDesc.BufferLocation);

	return indexSet.constantBufferIndices.back();
}

void Graphics::ComputeObject::SetSampler(size_t registerIndex, D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE addressU, D3D12_TEXTURE_ADDRESS_MODE addressV, D3D12_TEXTURE_ADDRESS_MODE addressW, uint32_t maxAnisotropy)
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

void Graphics::ComputeObject::AssignConstantBuffer(size_t registerIndex, ConstantBufferId constantBufferId)
{
	registerSet.constantBufferRegisterIndices.push_back(registerIndex);
	indexSet.constantBufferIndices.push_back(constantBufferId);

	constantBufferAddresses.push_back(resourceManager.GetConstantBuffer(constantBufferId).constantBufferViewDesc.BufferLocation);
}

void Graphics::ComputeObject::AssignTexture(size_t registerIndex, TextureId textureId)
{
	registerSet.textureRegisterIndices.push_back(registerIndex);
	indexSet.textureIndices.push_back(textureId);
}

void Graphics::ComputeObject::AssignTexture(size_t registerIndex, RWTextureId rwTextureId)
{
	registerSet.rwTextureReadOnlyRegisterIndices.push_back(registerIndex);
	indexSet.rwTextureReadOnlyIndices.push_back(rwTextureId);
}

void Graphics::ComputeObject::AssignRenderTexture(size_t registerIndex, RenderTargetId renderTargetId)
{
	registerSet.renderTextureRegisterIndices.push_back(registerIndex);
	indexSet.renderTargetIndices.push_back(renderTargetId);
}

void Graphics::ComputeObject::AssignDepthTexture(size_t registerIndex, DepthStencilId depthStencilId)
{
	registerSet.depthTextureRegisterIndices.push_back(registerIndex);
	indexSet.depthStencilIndices.push_back(depthStencilId);
}

void Graphics::ComputeObject::AssignBuffer(size_t registerIndex, BufferId bufferId)
{
	registerSet.bufferRegisterIndices.push_back(registerIndex);
	indexSet.bufferIndices.push_back(bufferId);
}

void Graphics::ComputeObject::AssignBuffer(size_t registerIndex, RWBufferId rwBufferId)
{
	registerSet.rwBufferReadOnlyRegisterIndices.push_back(registerIndex);
	indexSet.rwBufferReadOnlyIndices.push_back(rwBufferId);
}

void Graphics::ComputeObject::AssignRWTexture(size_t registerIndex, RWTextureId rwTextureId)
{
	registerSet.rwTextureRegisterIndices.push_back(registerIndex);
	indexSet.rwTextureIndices.push_back(rwTextureId);
}

void Graphics::ComputeObject::AssignRWBuffer(size_t registerIndex, RWBufferId rwBufferId)
{
	registerSet.rwBufferRegisterIndices.push_back(registerIndex);
	indexSet.rwBufferIndices.push_back(rwBufferId);
}

void Graphics::ComputeObject::AssignShader(D3D12_SHADER_BYTECODE _computeShader)
{
	computeShader = _computeShader;
}

void Graphics::ComputeObject::UpdateConstantBuffer(ConstantBufferId constantBufferId, void* bufferData, size_t bufferSize)
{
	resourceManager.UpdateConstantBuffer(constantBufferId, bufferData, bufferSize);
}

void Graphics::ComputeObject::Compose(ID3D12Device* device)
{
	if (isComposed)
	{
		rootSignature.Reset();
		pipelineState.Reset();

		isComposed = false;
	}

	std::vector<D3D12_DESCRIPTOR_RANGE> textureDescRanges;
	std::vector<D3D12_ROOT_DESCRIPTOR_TABLE> textureRootDescTables;

	CreateResourceRootDescriptorTables(registerSet, textureDescRanges, textureRootDescTables);
	
	std::vector<D3D12_ROOT_PARAMETER> rootParameters;
	CreateRootParameters(registerSet, textureRootDescTables, rootParameters);

	CreateRootSignature(device, rootParameters, samplerDescs, &rootSignature);

	CreateGraphicsPipelineState(device, rootSignature.Get(), computeShader, &pipelineState);

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

bool Graphics::ComputeObject::IsComposed() const noexcept
{
	return isComposed;
}

void Graphics::ComputeObject::SetThreadGroupCount(uint32_t xCount, uint32_t yCount, uint32_t zCount)
{
	threadGroupCountX = xCount;
	threadGroupCountY = yCount;
	threadGroupCountZ = zCount;
}

void Graphics::ComputeObject::Present(ID3D12GraphicsCommandList* commandList) const
{
	commandList->SetPipelineState(pipelineState.Get());
	commandList->SetComputeRootSignature(rootSignature.Get());

	uint32_t rootParameterIndex{};

	for (auto& constantBufferAddress : constantBufferAddresses)
		commandList->SetComputeRootConstantBufferView(rootParameterIndex++, constantBufferAddress);

	for (auto& firstResourceDescriptorBase : firstResourceDescriptorBases)
		commandList->SetComputeRootDescriptorTable(rootParameterIndex++, firstResourceDescriptorBase);

	commandList->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
}

void Graphics::ComputeObject::CreateResourceRootDescriptorTables(const RegisterSet& _registerSet, std::vector<D3D12_DESCRIPTOR_RANGE>& descriptorRanges,
	std::vector<D3D12_ROOT_DESCRIPTOR_TABLE>& rootDescriptorTables)
{
	descriptorRanges.clear();

	D3D12_DESCRIPTOR_RANGE descRange{};
	descRange.NumDescriptors = 1;
	descRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

	for (auto& textureRegisterId : _registerSet.textureRegisterIndices)
	{
		descRange.BaseShaderRegister = textureRegisterId;
		descriptorRanges.push_back(descRange);
	}

	for (auto& rwTextureRegisterId : _registerSet.rwTextureReadOnlyRegisterIndices)
	{
		descRange.BaseShaderRegister = rwTextureRegisterId;
		descriptorRanges.push_back(descRange);
	}

	for (auto& renderTextureRegisterId : _registerSet.renderTextureRegisterIndices)
	{
		descRange.BaseShaderRegister = renderTextureRegisterId;
		descriptorRanges.push_back(descRange);
	}

	for (auto& depthTextureRegisterId : _registerSet.depthTextureRegisterIndices)
	{
		descRange.BaseShaderRegister = depthTextureRegisterId;
		descriptorRanges.push_back(descRange);
	}

	for (auto& bufferRegisterId : _registerSet.bufferRegisterIndices)
	{
		descRange.BaseShaderRegister = bufferRegisterId;
		descriptorRanges.push_back(descRange);
	}

	for (auto& rwBufferRegisterId : _registerSet.rwBufferReadOnlyRegisterIndices)
	{
		descRange.BaseShaderRegister = rwBufferRegisterId;
		descriptorRanges.push_back(descRange);
	}

	descRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;

	for (auto& rwTextureRegisterId : _registerSet.rwTextureRegisterIndices)
	{
		descRange.BaseShaderRegister = rwTextureRegisterId;
		descriptorRanges.push_back(descRange);
	}

	for (auto& rwBufferRegisterId : _registerSet.rwBufferRegisterIndices)
	{
		descRange.BaseShaderRegister = rwBufferRegisterId;
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

void Graphics::ComputeObject::CreateRootParameters(const RegisterSet& _registerSet, const std::vector<D3D12_ROOT_DESCRIPTOR_TABLE>& rootDescriptorTables,
	std::vector<D3D12_ROOT_PARAMETER>& rootParameters)
{
	for (auto& constantBufferIndex : _registerSet.constantBufferRegisterIndices)
	{
		D3D12_ROOT_PARAMETER rootParameter{};

		rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParameter.Descriptor.RegisterSpace = 0;
		rootParameter.Descriptor.ShaderRegister = constantBufferIndex;
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

void Graphics::ComputeObject::CreateRootSignature(ID3D12Device* device, const std::vector<D3D12_ROOT_PARAMETER>& rootParameters, const std::vector<D3D12_STATIC_SAMPLER_DESC>& samplerDescs,
	ID3D12RootSignature** rootSignature)
{
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
	rootSignatureDesc.NumParameters = rootParameters.size();

	if (rootSignatureDesc.NumParameters > 0)
		rootSignatureDesc.pParameters = rootParameters.data();

	rootSignatureDesc.NumStaticSamplers = samplerDescs.size();

	if (rootSignatureDesc.NumStaticSamplers > 0)
		rootSignatureDesc.pStaticSamplers = samplerDescs.data();
	
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;

	ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signature, &error),
		"Material::CreateRootSignature: Root Signature serialization failed!");

	ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(rootSignature)),
		"Material::CreateRootSignature: Root Signature creating failed!");
}

void Graphics::ComputeObject::CreateGraphicsPipelineState(ID3D12Device* device, ID3D12RootSignature* rootSignature, D3D12_SHADER_BYTECODE _computeShader,
	ID3D12PipelineState** pipelineState)
{
	D3D12_COMPUTE_PIPELINE_STATE_DESC pipelineStateDesc{};
	pipelineStateDesc.CS = _computeShader;
	pipelineStateDesc.pRootSignature = rootSignature;

	ThrowIfFailed(device->CreateComputePipelineState(&pipelineStateDesc, IID_PPV_ARGS(pipelineState)),
		"ComputeObject::CreateComputePipelineState: Compute Pipeline State creating failed!");
}
