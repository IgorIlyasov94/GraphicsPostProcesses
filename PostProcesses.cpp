#include "PostProcesses.h"
#include "Resources/Shaders/ScreenQuad.vsh.h"
#include "Resources/Shaders/GaussianBlurX.psh.h"
#include "Resources/Shaders/GaussianBlurY.psh.h"
#include "Resources/Shaders/HDRBrightPass.psh.h"
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
	sceneViewportHalf = _sceneViewport;
	sceneViewportHalf.Width /= 2.0f;
	sceneViewportHalf.Height /= 2.0f;

	sceneScissorRect = _sceneScissorRect;
	sceneScissorRectHalf = _sceneScissorRect;
	sceneScissorRectHalf.right /= 2;
	sceneScissorRectHalf.bottom /= 2;

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

	for (uint32_t renderTargetId = 0; renderTargetId < INTERMEDIATE_8B_RENDER_TARGET_COUNT; renderTargetId++)
	{
		intermediate8bTargetId[renderTargetId] = resourceManager.CreateRenderTarget(GraphicsSettings::GetResolutionX(), GraphicsSettings::GetResolutionY(),
			DXGI_FORMAT_R8G8B8A8_UNORM);

		intermediate8bTargetDescriptor[renderTargetId] = resourceManager.GetRenderTargetDescriptorBase(intermediate8bTargetId[renderTargetId]);
	}

	for (uint32_t renderTargetId = 0; renderTargetId < INTERMEDIATE_8B_HALF_RENDER_TARGET_COUNT; renderTargetId++)
	{
		intermediate8bHalfTargetId[renderTargetId] = resourceManager.CreateRenderTarget(GraphicsSettings::GetResolutionX() / 2, GraphicsSettings::GetResolutionY() / 2,
			DXGI_FORMAT_R8G8B8A8_UNORM);

		intermediate8bHalfTargetDescriptor[renderTargetId] = resourceManager.GetRenderTargetDescriptorBase(intermediate8bHalfTargetId[renderTargetId]);
	}

	for (uint32_t renderTargetId = 0; renderTargetId < INTERMEDIATE_16B_RENDER_TARGET_COUNT; renderTargetId++)
	{
		intermediate16bTargetId[renderTargetId] = resourceManager.CreateRenderTarget(GraphicsSettings::GetResolutionX(), GraphicsSettings::GetResolutionY(),
			DXGI_FORMAT_R16G16B16A16_FLOAT);

		intermediate16bTargetDescriptor[renderTargetId] = resourceManager.GetRenderTargetDescriptorBase(intermediate16bTargetId[renderTargetId]);
	}

	for (uint32_t renderTargetId = 0; renderTargetId < INTERMEDIATE_32B_RENDER_TARGET_COUNT; renderTargetId++)
	{
		intermediate32bTargetId[renderTargetId] = resourceManager.CreateRenderTarget(GraphicsSettings::GetResolutionX(), GraphicsSettings::GetResolutionY(),
			DXGI_FORMAT_R32G32B32A32_FLOAT);

		intermediate32bTargetDescriptor[renderTargetId] = resourceManager.GetRenderTargetDescriptorBase(intermediate32bTargetId[renderTargetId]);
	}
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

		{
			brightPassMaterial = std::make_shared<Material>();
			brightPassMaterial->SetVertexFormat(VertexFormat::POSITION_TEXCOORD);
			brightPassMaterial->AssignRenderTexture(commandList, 0, hdrInputRenderTargetId);
			brightPassMaterial->SetVertexShader({ quadVertexShader, sizeof(quadVertexShader) });
			brightPassMaterial->SetPixelShader({ brightPassPixelShader, sizeof(brightPassPixelShader) });
			brightPassMaterial->SetRenderTargetFormat(0, DXGI_FORMAT_R8G8B8A8_UNORM);

			hdrConstantBufferId = brightPassMaterial->SetConstantBuffer(0, &hdrConstantBuffer, sizeof(HDRConstantBuffer));

			brightPassMaterial->Compose(device);

			brightPass = std::make_shared<GraphicObject>();
			brightPass->AssignMaterial(brightPassMaterial.get());
			brightPass->AssignMesh(screenQuadMesh.get());
		}

		{
			gaussianBlurXMaterial = std::make_shared<Material>();
			gaussianBlurXMaterial->SetVertexFormat(VertexFormat::POSITION_TEXCOORD);
			gaussianBlurXMaterial->AssignRenderTexture(commandList, 0, intermediate8bHalfTargetId[0]);
			gaussianBlurXMaterial->SetVertexShader({ quadVertexShader, sizeof(quadVertexShader) });
			gaussianBlurXMaterial->SetPixelShader({ gaussianBlurXPixelShader, sizeof(gaussianBlurXPixelShader) });
			gaussianBlurXMaterial->SetRenderTargetFormat(0, DXGI_FORMAT_R8G8B8A8_UNORM);

			gaussianBlurXMaterial->Compose(device);

			gaussianBlurX = std::make_shared<GraphicObject>();
			gaussianBlurX->AssignMaterial(gaussianBlurXMaterial.get());
			gaussianBlurX->AssignMesh(screenQuadMesh.get());
		}

		{
			gaussianBlurYMaterial = std::make_shared<Material>();
			gaussianBlurYMaterial->SetVertexFormat(VertexFormat::POSITION_TEXCOORD);
			gaussianBlurYMaterial->AssignRenderTexture(commandList, 0, intermediate8bHalfTargetId[1]);
			gaussianBlurYMaterial->SetVertexShader({ quadVertexShader, sizeof(quadVertexShader) });
			gaussianBlurYMaterial->SetPixelShader({ gaussianBlurYPixelShader, sizeof(gaussianBlurYPixelShader) });
			gaussianBlurYMaterial->SetRenderTargetFormat(0, DXGI_FORMAT_R8G8B8A8_UNORM);

			gaussianBlurYMaterial->Compose(device);

			gaussianBlurY = std::make_shared<GraphicObject>();
			gaussianBlurY->AssignMaterial(gaussianBlurYMaterial.get());
			gaussianBlurY->AssignMesh(screenQuadMesh.get());
		}

		{
			toneMappingMaterial = std::make_shared<Material>();
			toneMappingMaterial->SetVertexFormat(VertexFormat::POSITION_TEXCOORD);
			toneMappingMaterial->AssignRenderTexture(commandList, 0, hdrInputRenderTargetId);
			toneMappingMaterial->AssignRenderTexture(commandList, 1, intermediate8bHalfTargetId[0]);
			toneMappingMaterial->SetVertexShader({ quadVertexShader, sizeof(quadVertexShader) });
			toneMappingMaterial->SetPixelShader({ toneMappingPixelShader, sizeof(toneMappingPixelShader) });
			toneMappingMaterial->SetRenderTargetFormat(0, DXGI_FORMAT_R8G8B8A8_UNORM);
			toneMappingMaterial->AssignConstantBuffer(0, hdrConstantBufferId);

			toneMappingMaterial->Compose(device);

			toneMapping = std::make_shared<GraphicObject>();

			toneMapping->AssignMaterial(toneMappingMaterial.get());
			toneMapping->AssignMesh(screenQuadMesh.get());
		}
	}

	isComposed = true;
}

void Graphics::PostProcesses::UpdateHDR(ID3D12GraphicsCommandList* commandList, float3 shiftVector, float middleGray, float whiteCutoff, float brightPassThreshold,
	float brightPassOffset)
{
	hdrConstantBuffer.shiftVector = shiftVector;
	hdrConstantBuffer.middleGray = middleGray;
	hdrConstantBuffer.whiteCutoff = whiteCutoff;
	hdrConstantBuffer.brightPassOffset = brightPassThreshold;
	hdrConstantBuffer.brightPassThreshold = brightPassOffset;

	if (isComposed)
		toneMappingMaterial->UpdateConstantBuffer(hdrConstantBufferId, &hdrConstantBuffer, sizeof(hdrConstantBuffer));
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
	commandList->RSSetViewports(1, &sceneViewportHalf);
	commandList->RSSetScissorRects(1, &sceneScissorRectHalf);

	{
		commandList->OMSetRenderTargets(1, &intermediate8bHalfTargetDescriptor[0], false, nullptr);
		brightPass->Draw(commandList);
	}

	{
		SetResourceBarrier(commandList, resourceManager.GetRenderTarget(intermediate8bHalfTargetId[0]).textureAllocation.textureResource, D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		commandList->OMSetRenderTargets(1, &intermediate8bHalfTargetDescriptor[1], false, nullptr);
		gaussianBlurX->Draw(commandList);

		SetResourceBarrier(commandList, resourceManager.GetRenderTarget(intermediate8bHalfTargetId[0]).textureAllocation.textureResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET);
	}

	{
		SetResourceBarrier(commandList, resourceManager.GetRenderTarget(intermediate8bHalfTargetId[1]).textureAllocation.textureResource, D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		commandList->OMSetRenderTargets(1, &intermediate8bHalfTargetDescriptor[0], false, nullptr);
		gaussianBlurY->Draw(commandList);

		SetResourceBarrier(commandList, resourceManager.GetRenderTarget(intermediate8bHalfTargetId[1]).textureAllocation.textureResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET);
	}

	commandList->RSSetViewports(1, &sceneViewport);
	commandList->RSSetScissorRects(1, &sceneScissorRect);

	{
		SetResourceBarrier(commandList, resourceManager.GetRenderTarget(intermediate8bHalfTargetId[0]).textureAllocation.textureResource, D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		commandList->OMSetRenderTargets(1, destRenderTargetDescriptor, false, nullptr);
		toneMapping->Draw(commandList);

		SetResourceBarrier(commandList, resourceManager.GetRenderTarget(intermediate8bHalfTargetId[0]).textureAllocation.textureResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET);
	}
}
