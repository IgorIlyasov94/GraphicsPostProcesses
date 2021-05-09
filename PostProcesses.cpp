#include "PostProcesses.h"
#include "Resources/Shaders/ScreenQuad.vsh.h"
#include "Resources/Shaders/HDRToneMapping.psh.h"

Graphics::PostProcesses::PostProcesses()
	: sceneViewport{}, isHDREnabled{}, isComposed{}, hdrConstantBuffer{}
{

}

Graphics::PostProcesses& Graphics::PostProcesses::GetInstance()
{
	static PostProcesses instance;

	return instance;
}

void Graphics::PostProcesses::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const int32_t& resolutionX, const int32_t& resolutionY,
	const D3D12_VIEWPORT& _sceneViewport, const D3D12_RECT& _sceneScissorRect, const RenderTargetId& _sceneRenderTargetId)
{
	sceneViewport = _sceneViewport;
	sceneScissorRect = _sceneScissorRect;
	sceneRenderTargetId = _sceneRenderTargetId;
	sceneRenderTargetDescriptor = resourceManager.GetRenderTargetDescriptorBase(sceneRenderTargetId);

	noiseTextureId = resourceManager.CreateTexture("Resources\\Textures\\Noise.dds");
	diffuseTextureId = resourceManager.CreateTexture("Resources\\Textures\\Diffuse0.dds");
	
	ScreenQuadVertex vertices[] =
	{
		{ float3(-1.0f, 1.0f, 0.5f), float2(0.0f, 0.0f) },
		{ float3(1.0f, 1.0f, 0.5f), float2(1.0f, 0.0f) },
		{ float3(-1.0f, -1.0f, 0.5f), float2(0.0f, 1.0f) },
		{ float3(1.0f, -1.0f, 0.5f), float2(1.0f, 1.0f) }
	};

	uint32_t indices[] =
	{
		0, 1, 2,
		2, 1, 3
	};

	screenQuadMesh = std::make_shared<Mesh>(VertexFormat::POSITION_TEXCOORD, &vertices, sizeof(vertices), &indices, sizeof(indices));
}

void Graphics::PostProcesses::EnableHDR(float3 shiftVector, float middleGray, float whiteCutoff,
	float brightPassThreshold, float brightPassOffset)
{
	isHDREnabled = true;

	hdrConstantBuffer.shiftVector = shiftVector;
	hdrConstantBuffer.middleGray = middleGray;
	hdrConstantBuffer.whiteCutoff = whiteCutoff;
	hdrConstantBuffer.brightPassOffset = brightPassThreshold;
	hdrConstantBuffer.brightPassThreshold = brightPassOffset;
}

void Graphics::PostProcesses::Compose(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
	if (isHDREnabled)
	{
		hdrInputRenderTargetId = sceneRenderTargetId;

		hdrPostProcessMaterial = std::make_shared<Material>();

		hdrPostProcessMaterial->SetVertexFormat(VertexFormat::POSITION_TEXCOORD);
		hdrPostProcessMaterial->AssignRenderTexture(commandList, 0, sceneRenderTargetId);
		hdrPostProcessMaterial->AssignTexture(commandList, 1, diffuseTextureId, true);
		hdrPostProcessMaterial->SetVertexShader({ quadVertexShader, sizeof(quadVertexShader) });
		hdrPostProcessMaterial->SetPixelShader({ toneMappingPixelShader, sizeof(toneMappingPixelShader) });
		hdrPostProcessMaterial->SetRenderTargetFormat(0, DXGI_FORMAT_R8G8B8A8_UNORM);

		hdrConstantBufferId = hdrPostProcessMaterial->SetConstantBuffer(0, &hdrConstantBuffer, sizeof(HDRConstantBuffer));

		hdrPostProcessMaterial->Compose(device);

		hdrPostProcess = std::make_shared<GraphicObject>();

		hdrPostProcess->AssignMaterial(hdrPostProcessMaterial.get());
		hdrPostProcess->AssignMesh(screenQuadMesh.get());
	}

	isComposed = true;
}

void Graphics::PostProcesses::PresentProcessChain(ID3D12GraphicsCommandList* commandList, const D3D12_CPU_DESCRIPTOR_HANDLE* destRenderTargetDescriptor)
{
	if (!isComposed)
		return;

	if (isHDREnabled)
		ProcessHDR(commandList, hdrInputRenderTargetId, destRenderTargetDescriptor);
}

void Graphics::PostProcesses::ProcessHDR(ID3D12GraphicsCommandList* commandList, const RenderTargetId& srcRenderTargetId,
	const D3D12_CPU_DESCRIPTOR_HANDLE* destRenderTargetDescriptor)
{
	commandList->RSSetViewports(1, &sceneViewport);
	commandList->RSSetScissorRects(1, &sceneScissorRect);
	
	commandList->OMSetRenderTargets(1, destRenderTargetDescriptor, false, nullptr);

	hdrConstantBuffer.shiftVector.x *= 1.01f;
	hdrConstantBuffer.shiftVector.y *= 1.01f;
	hdrConstantBuffer.shiftVector.z *= 1.01f;

	hdrPostProcessMaterial->UpdateConstantBuffer(hdrConstantBufferId, &hdrConstantBuffer, sizeof(hdrConstantBuffer));
	hdrPostProcess->Draw(commandList);
}
