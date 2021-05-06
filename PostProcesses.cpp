#include "PostProcesses.h"
#include "Resources/Shaders/ScreenQuad.vsh.h"
#include "Resources/Shaders/HDRToneMapping.psh.h"

Graphics::PostProcesses::PostProcesses()
	: sceneViewport{}, hdrConstantBuffer{}
{

}

Graphics::PostProcesses& Graphics::PostProcesses::GetInstance()
{
	static PostProcesses instance;

	return instance;
}

void Graphics::PostProcesses::Initialize(const int32_t& resolutionX, const int32_t& resolutionY, ID3D12Device* device, const D3D12_VIEWPORT& _sceneViewport,
	const D3D12_RECT& _sceneScissorRect, ID3D12GraphicsCommandList* commandList)
{
	sceneViewport = _sceneViewport;
	sceneScissorRect = _sceneScissorRect;

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
	
	hdrPostProcessMaterial = std::make_shared<Material>();

	hdrPostProcessMaterial->SetVertexFormat(VertexFormat::POSITION_TEXCOORD);
	hdrPostProcessMaterial->AssignTexture(commandList, 0, noiseTextureId, true);
	hdrPostProcessMaterial->AssignTexture(commandList, 1, diffuseTextureId, true);
	hdrPostProcessMaterial->SetVertexShader({ quadVertexShader, sizeof(quadVertexShader) });
	hdrPostProcessMaterial->SetPixelShader({ toneMappingPixelShader, sizeof(toneMappingPixelShader) });
	hdrPostProcessMaterial->SetRenderTargetFormat(0, DXGI_FORMAT_R8G8B8A8_UNORM);
	
	hdrConstantBuffer.shiftVector = { 0.08f, 0.06f, 0.07f };
	hdrConstantBuffer.middleGray = 0.6f;
	hdrConstantBuffer.whiteCutoff = 0.8f;
	hdrConstantBuffer.brightPassOffset = 5.0f;
	hdrConstantBuffer.brightPassThreshold = 10.0f;

	hdrConstantBufferId = hdrPostProcessMaterial->SetConstantBuffer(0, &hdrConstantBuffer, sizeof(HDRConstantBuffer));

	hdrPostProcessMaterial->Compose(device);

	hdrPostProcess = std::make_shared<GraphicObject>();

	hdrPostProcess->AssignMaterial(hdrPostProcessMaterial.get());
	hdrPostProcess->AssignMesh(screenQuadMesh.get());
}

void Graphics::PostProcesses::EnableHDR(ID3D12GraphicsCommandList* commandList, size_t bufferIndex)
{
	ID3D12DescriptorHeap* descHeaps[] = { resourceManager.GetTexture(noiseTextureId).descriptorAllocation.descriptorHeap };

	commandList->SetDescriptorHeaps(_countof(descHeaps), descHeaps);

	commandList->RSSetViewports(1, &sceneViewport);
	commandList->RSSetScissorRects(1, &sceneScissorRect);
	
	commandList->OMSetRenderTargets(1, &resourceManager.GetSwapChainDescriptorBase(bufferIndex), false, nullptr);

	hdrConstantBuffer.shiftVector.x *= 1.01f;
	hdrConstantBuffer.shiftVector.y *= 1.01f;
	hdrConstantBuffer.shiftVector.z *= 1.01f;

	hdrPostProcessMaterial->UpdateConstantBuffer(hdrConstantBufferId, &hdrConstantBuffer, sizeof(hdrConstantBuffer));
	hdrPostProcess->Draw(commandList);
}
