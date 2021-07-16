#include "PostProcesses.h"
#include "Resources/Shaders/ScreenQuadVS.hlsl.h"
#include "Resources/Shaders/AntiAliasingPS.hlsl.h"
#include "Resources/Shaders/GaussianBlurXPS.hlsl.h"
#include "Resources/Shaders/GaussianBlurYPS.hlsl.h"
#include "Resources/Shaders/HDRBrightPassPS.hlsl.h"
#include "Resources/Shaders/HDRToneMappingPS.hlsl.h"

Graphics::PostProcesses::PostProcesses()
	: sceneViewport{}, isAAEnabled{}, isHDREnabled{}, isComposed{}, aaConstantBuffer{}, hdrConstantBuffer{}
{

}

Graphics::PostProcesses& Graphics::PostProcesses::GetInstance()
{
	static PostProcesses instance;

	return instance;
}

void Graphics::PostProcesses::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, const int32_t& resolutionX, const int32_t& resolutionY,
	const D3D12_VIEWPORT& _sceneViewport, const D3D12_RECT& _sceneScissorRect, const RenderTargetId& _sceneRenderTargetId, const RenderTargetId& _normalRenderTargetId,
	const DepthStencilId& _sceneDepthStencilId)
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
	normalRenderTargetId = _normalRenderTargetId;
	sceneDepthStencilId = _sceneDepthStencilId;
	sceneRenderTargetDescriptor = resourceManager.GetRenderTargetDescriptorBase(sceneRenderTargetId);
	normalRenderTargetDescriptor = resourceManager.GetRenderTargetDescriptorBase(normalRenderTargetId);
	sceneDepthStencilDescriptor = resourceManager.GetDepthStencilDescriptorBase(sceneDepthStencilId);

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

	screenQuadMesh = std::shared_ptr<Mesh>(new Mesh(VertexFormat::POSITION_TEXCOORD, &vertices, sizeof(vertices), &indices, sizeof(indices)));

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

	for (uint32_t renderTargetId = 0; renderTargetId < INTERMEDIATE_8B_QUART_RENDER_TARGET_COUNT; renderTargetId++)
	{
		intermediate8bQuartTargetId[renderTargetId] = resourceManager.CreateRenderTarget(GraphicsSettings::GetResolutionX() / 4, GraphicsSettings::GetResolutionY() / 4,
			DXGI_FORMAT_R8G8B8A8_UNORM);

		intermediate8bQuartTargetDescriptor[renderTargetId] = resourceManager.GetRenderTargetDescriptorBase(intermediate8bQuartTargetId[renderTargetId]);
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

void Graphics::PostProcesses::EnableAA()
{
	isAAEnabled = true;

	aaConstantBuffer.pixelSize = float2(1.0f / GraphicsSettings::GetResolutionX(), 1.0f / GraphicsSettings::GetResolutionY());
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
	auto srcRenderTarget = sceneRenderTargetId;
	
	if (isAAEnabled)
	{
		antiAliasingMaterial = std::make_shared<Material>();
		antiAliasingMaterial->SetVertexFormat(VertexFormat::POSITION_TEXCOORD);
		antiAliasingMaterial->AssignRenderTexture(0, srcRenderTarget);
		antiAliasingMaterial->AssignRenderTexture(1, normalRenderTargetId);
		antiAliasingMaterial->AssignDepthTexture(2, sceneDepthStencilId);
		antiAliasingMaterial->SetVertexShader({ screenQuadVS, sizeof(screenQuadVS) });
		antiAliasingMaterial->SetPixelShader({ antiAliasingPS, sizeof(antiAliasingPS) });
		antiAliasingMaterial->SetRenderTargetFormat(0, (isHDREnabled) ? DXGI_FORMAT_R16G16B16A16_FLOAT : DXGI_FORMAT_R8G8B8A8_UNORM);
		antiAliasingMaterial->SetConstantBuffer(0, &aaConstantBuffer, sizeof(AAConstantBuffer));
		antiAliasingMaterial->SetSampler(0, D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 1);

		antiAliasingMaterial->Compose(device);

		antiAliasing = std::make_shared<GraphicObject>();
		antiAliasing->AssignMaterial(antiAliasingMaterial.get());
		antiAliasing->AssignRenderableEntity(screenQuadMesh.get());

		aaSrcRenderTargetId = srcRenderTarget;
		aaDestRenderTargetDescriptor = &intermediate16bTargetDescriptor[0];
		srcRenderTarget = intermediate16bTargetId[0];
		finalDestRenderTargetDescriptor = aaDestRenderTargetDescriptor;
	}

	if (isHDREnabled)
	{
		{
			brightPassMaterial = std::make_shared<Material>();
			brightPassMaterial->SetVertexFormat(VertexFormat::POSITION_TEXCOORD);
			brightPassMaterial->AssignRenderTexture(0, srcRenderTarget);
			brightPassMaterial->SetVertexShader({ screenQuadVS, sizeof(screenQuadVS) });
			brightPassMaterial->SetPixelShader({ brightPassPS, sizeof(brightPassPS) });
			brightPassMaterial->SetRenderTargetFormat(0, DXGI_FORMAT_R8G8B8A8_UNORM);

			hdrConstantBufferId = brightPassMaterial->SetConstantBuffer(0, &hdrConstantBuffer, sizeof(HDRConstantBuffer));

			brightPassMaterial->Compose(device);

			brightPass = std::make_shared<GraphicObject>();
			brightPass->AssignMaterial(brightPassMaterial.get());
			brightPass->AssignRenderableEntity(screenQuadMesh.get());
		}

		{
			gaussianBlurXMaterial = std::make_shared<Material>();
			gaussianBlurXMaterial->SetVertexFormat(VertexFormat::POSITION_TEXCOORD);
			gaussianBlurXMaterial->AssignRenderTexture(0, intermediate8bQuartTargetId[0]);
			gaussianBlurXMaterial->SetVertexShader({ screenQuadVS, sizeof(screenQuadVS) });
			gaussianBlurXMaterial->SetPixelShader({ gaussianBlurXPS, sizeof(gaussianBlurXPS) });
			gaussianBlurXMaterial->SetRenderTargetFormat(0, DXGI_FORMAT_R8G8B8A8_UNORM);

			gaussianBlurXMaterial->Compose(device);

			gaussianBlurX = std::make_shared<GraphicObject>();
			gaussianBlurX->AssignMaterial(gaussianBlurXMaterial.get());
			gaussianBlurX->AssignRenderableEntity(screenQuadMesh.get());
		}

		{
			gaussianBlurYMaterial = std::make_shared<Material>();
			gaussianBlurYMaterial->SetVertexFormat(VertexFormat::POSITION_TEXCOORD);
			gaussianBlurYMaterial->AssignRenderTexture(0, intermediate8bQuartTargetId[1]);
			gaussianBlurYMaterial->SetVertexShader({ screenQuadVS, sizeof(screenQuadVS) });
			gaussianBlurYMaterial->SetPixelShader({ gaussianBlurYPS, sizeof(gaussianBlurYPS) });
			gaussianBlurYMaterial->SetRenderTargetFormat(0, DXGI_FORMAT_R8G8B8A8_UNORM);

			gaussianBlurYMaterial->Compose(device);

			gaussianBlurY = std::make_shared<GraphicObject>();
			gaussianBlurY->AssignMaterial(gaussianBlurYMaterial.get());
			gaussianBlurY->AssignRenderableEntity(screenQuadMesh.get());
		}

		{
			toneMappingMaterial = std::make_shared<Material>();
			toneMappingMaterial->SetVertexFormat(VertexFormat::POSITION_TEXCOORD);
			toneMappingMaterial->AssignRenderTexture(0, srcRenderTarget);
			toneMappingMaterial->AssignRenderTexture(1, intermediate8bQuartTargetId[0]);
			toneMappingMaterial->SetVertexShader({ screenQuadVS, sizeof(screenQuadVS) });
			toneMappingMaterial->SetPixelShader({ toneMappingPS, sizeof(toneMappingPS) });
			toneMappingMaterial->SetRenderTargetFormat(0, DXGI_FORMAT_R8G8B8A8_UNORM);
			toneMappingMaterial->AssignConstantBuffer(0, hdrConstantBufferId);
			toneMappingMaterial->SetSampler(0, D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 1);

			toneMappingMaterial->Compose(device);

			toneMapping = std::make_shared<GraphicObject>();

			toneMapping->AssignMaterial(toneMappingMaterial.get());
			toneMapping->AssignRenderableEntity(screenQuadMesh.get());
		}

		hdrSrcRenderTargetId = srcRenderTarget;
		hdrDestRenderTargetDescriptor = &intermediate8bTargetDescriptor[0];
		srcRenderTarget = intermediate16bTargetId[0];
		finalDestRenderTargetDescriptor = hdrDestRenderTargetDescriptor;
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

	resourceManager.SetResourceBarrier(commandList, sceneDepthStencilId, D3D12_RESOURCE_BARRIER_FLAG_NONE,
		D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	*finalDestRenderTargetDescriptor = *destRenderTargetDescriptor;

	if (isAAEnabled)
		ProcessAA(commandList, aaSrcRenderTargetId, aaDestRenderTargetDescriptor);

	if (isHDREnabled)
		ProcessHDR(commandList, hdrSrcRenderTargetId, hdrDestRenderTargetDescriptor);

	resourceManager.SetResourceBarrier(commandList, sceneDepthStencilId, D3D12_RESOURCE_BARRIER_FLAG_NONE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
}

void Graphics::PostProcesses::ProcessAA(ID3D12GraphicsCommandList* commandList, const RenderTargetId& srcRenderTargetId, const D3D12_CPU_DESCRIPTOR_HANDLE* destRenderTargetDescriptor)
{
	commandList->RSSetViewports(1, &sceneViewport);
	commandList->RSSetScissorRects(1, &sceneScissorRect);

	{
		auto beforeResourceState = resourceManager.GetRenderTarget(srcRenderTargetId).currentResourceState;
		resourceManager.SetResourceBarrier(commandList, srcRenderTargetId, D3D12_RESOURCE_BARRIER_FLAG_NONE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		commandList->OMSetRenderTargets(1, destRenderTargetDescriptor, false, nullptr);
		antiAliasing->Draw(commandList);

		resourceManager.SetResourceBarrier(commandList, srcRenderTargetId, D3D12_RESOURCE_BARRIER_FLAG_NONE, beforeResourceState);
	}
}

void Graphics::PostProcesses::ProcessHDR(ID3D12GraphicsCommandList* commandList, const RenderTargetId& srcRenderTargetId, const D3D12_CPU_DESCRIPTOR_HANDLE* destRenderTargetDescriptor)
{
	commandList->RSSetViewports(1, &sceneViewportHalf);
	commandList->RSSetScissorRects(1, &sceneScissorRectHalf);

	{
		resourceManager.SetResourceBarrier(commandList, srcRenderTargetId, D3D12_RESOURCE_BARRIER_FLAG_NONE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		commandList->OMSetRenderTargets(1, &intermediate8bQuartTargetDescriptor[0], false, nullptr);
		brightPass->Draw(commandList);
	}

	{
		resourceManager.SetResourceBarrier(commandList, intermediate8bQuartTargetId[0], D3D12_RESOURCE_BARRIER_FLAG_NONE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		commandList->OMSetRenderTargets(1, &intermediate8bQuartTargetDescriptor[1], false, nullptr);
		gaussianBlurX->Draw(commandList);

		resourceManager.SetResourceBarrier(commandList, intermediate8bQuartTargetId[0], D3D12_RESOURCE_BARRIER_FLAG_NONE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}

	{
		resourceManager.SetResourceBarrier(commandList, intermediate8bQuartTargetId[1], D3D12_RESOURCE_BARRIER_FLAG_NONE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		commandList->OMSetRenderTargets(1, &intermediate8bQuartTargetDescriptor[0], false, nullptr);
		gaussianBlurY->Draw(commandList);

		resourceManager.SetResourceBarrier(commandList, intermediate8bQuartTargetId[1], D3D12_RESOURCE_BARRIER_FLAG_NONE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}

	commandList->RSSetViewports(1, &sceneViewport);
	commandList->RSSetScissorRects(1, &sceneScissorRect);

	{
		resourceManager.SetResourceBarrier(commandList, intermediate8bQuartTargetId[0], D3D12_RESOURCE_BARRIER_FLAG_NONE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		commandList->OMSetRenderTargets(1, destRenderTargetDescriptor, false, nullptr);
		toneMapping->Draw(commandList);

		resourceManager.SetResourceBarrier(commandList, intermediate8bQuartTargetId[0], D3D12_RESOURCE_BARRIER_FLAG_NONE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}

	resourceManager.SetResourceBarrier(commandList, srcRenderTargetId, D3D12_RESOURCE_BARRIER_FLAG_NONE, D3D12_RESOURCE_STATE_RENDER_TARGET);
}
