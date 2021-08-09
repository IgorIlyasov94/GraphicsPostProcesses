#include "SceneManager.h"
#include "Resources/Shaders/MeshStandardVS.hlsl.h"
#include "Resources/Shaders/MeshStandardPS.hlsl.h"
#include "Resources/Shaders/ParticleStandardVS.hlsl.h"
#include "Resources/Shaders/ParticleStandardGS.hlsl.h"
#include "Resources/Shaders/ParticleStandardPS.hlsl.h"
#include "Resources/Shaders/UISpriteStandardVS.hlsl.h"
#include "Resources/Shaders/UISpriteStandardPS.hlsl.h"
#include "Resources/Shaders/UITextStandardVS.hlsl.h"
#include "Resources/Shaders/UITextStandardPS.hlsl.h"

Graphics::SceneManager& Graphics::SceneManager::GetInstance()
{
	static SceneManager instance;

	return instance;
}

Graphics::Scene* Graphics::SceneManager::CreateNewScene()
{
	scenes.push_back(Scene());

	return &scenes.back();
}

void Graphics::SceneManager::InitializeTestScene(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
{
	const float fovAngleY = XM_PI / 4.0f;
	const float aspectRatio = GraphicsSettings::GetResolutionX() / static_cast<float>(GraphicsSettings::GetResolutionY());
	const float nearZ = 0.01f;
	const float farZ = 1024.0f;

	camera = std::make_shared<Camera>(fovAngleY, aspectRatio, nearZ, farZ);

	camera->Move({ 0.0f, 0.0f, -10.0f });
	camera->LookAt({ 0.0f, 0.5f, 0.0f });

	camera->Update();

	auto paddingDefaultTextureId = resourceManager.CreateTexture("Resources\\Textures\\PaddingDefaultTexture.dds");

	immutableGlobalConstBuffer.projection = camera->GetProjection();
	immutableGlobalConstBuffer.invProjection = camera->GetInvProjection();
	immutableGlobalConstBuffer.zNear = nearZ;
	immutableGlobalConstBuffer.zFar = farZ;
	immutableGlobalConstBuffer.zLinearizeCoeff = { (1.0f - farZ / nearZ) / farZ, 1.0f / nearZ };

	immutableGlobalConstBufferId = resourceManager.CreateConstantBuffer(&immutableGlobalConstBuffer, sizeof(immutableGlobalConstBuffer));
	globalConstBufferId = resourceManager.CreateConstantBuffer(&globalConstBuffer, sizeof(globalConstBuffer));

	currentScene = CreateNewScene();

	currentScene->GetLightingSystem()->CreatePointLight(float3(0.0f, 10.0f, 0.0f), float3(0.0f, 1.0f, 0.0f), 15.0f, 1.0f, false);
	currentScene->GetLightingSystem()->CreatePointLight(float3(4.0f, 5.0f, 0.0f), float3(1.0f, 0.0f, 0.0f), 4.0f, 1.0f, false);
	currentScene->GetLightingSystem()->CreatePointLight(float3(-4.0f, 5.0f, 0.0f), float3(0.0f, 0.0f, 1.0f), 5.0f, 1.0f, false);
	
	currentScene->GetLightingSystem()->ComposeLightBuffer(device, commandList, immutableGlobalConstBufferId, globalConstBufferId);
	
	goldenFrameMesh = std::shared_ptr<Mesh>(new Mesh("Resources\\Meshes\\GoldenFrame.obj", VertexFormat::POSITION | VertexFormat::NORMAL | VertexFormat::TEXCOORD, false, true));
	goldenFrameMaterial = std::make_shared<Material>();

	goldenFrameMaterial->SetVertexFormat(goldenFrameMesh->GetVertexFormat());
	goldenFrameMaterial->SetVertexShader({ meshStandardVS, sizeof(meshStandardVS) });
	goldenFrameMaterial->SetPixelShader({ meshStandardPS, sizeof(meshStandardPS) });
	goldenFrameMaterial->SetDepthTest(true);
	goldenFrameMaterial->SetDepthStencilFormat(32);
	goldenFrameMaterial->SetCullMode(D3D12_CULL_MODE_BACK);
	goldenFrameMaterial->SetRenderTargetFormat(0, DXGI_FORMAT_R16G16B16A16_FLOAT);
	goldenFrameMaterial->SetRenderTargetFormat(1, DXGI_FORMAT_R8G8B8A8_SNORM);
	goldenFrameMaterial->AssignConstantBuffer(0, immutableGlobalConstBufferId);
	goldenFrameMaterial->AssignConstantBuffer(1, globalConstBufferId);

	goldenFrameConstBuffer.world = XMMatrixIdentity();
	goldenFrameConstBuffer.wvp = camera->GetViewProjection();

	goldenFrameConstBufferId = goldenFrameMaterial->SetConstantBuffer(2, &goldenFrameConstBuffer, sizeof(goldenFrameConstBuffer));
	
	goldenFrameMaterial->AssignBuffer(0, currentScene->GetLightingSystem()->GetLightBufferId());
	goldenFrameMaterial->AssignTexture(1, currentScene->GetLightingSystem()->GetLightClusterId());
	
	goldenFrameMaterial->Compose(device);

	goldenFrame = std::make_shared<GraphicObject>();
	goldenFrame->SetRenderingLayer(RenderingLayer::RENDERING_LAYER_OPAQUE);
	goldenFrame->AssignRenderableEntity(goldenFrameMesh.get());
	goldenFrame->AssignMaterial(goldenFrameMaterial.get());

	testEffectParticleSystem = std::shared_ptr<ParticleSystem>(new ParticleSystem(100, 30, 600, paddingDefaultTextureId));
	testEffectParticleSystem->SetEmitter({ 0.0f, 2.0f, 0.0f }, 10.0f, 1, false, ParticleEmitterShape::EMITTER_POINT,
		BoundingBox({ { -3.0f, -3.0f, -3.0f }, { 3.0f, 3.0f, 3.0f } }));
	testEffectParticleSystem->SetSize({ 1.0f, 1.0f }, { 1.0f, 1.0f }, ParticleAnimationType::ANIMATION_NONE, {});
	testEffectParticleSystem->SetVelocity({ 0.0f, 1.0f, 0.0f }, { 0.0f, 5.0f, 0.0f }, 1.0f, ParticleAnimationType::ANIMATION_LERP, {});
	testEffectParticleSystem->SetColor({ 10.0f, 10.0f, 10.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }, ParticleAnimationType::ANIMATION_LERP, {});
	testEffectParticleSystem->SetAngularMotion(0.0f, XM_2PI, 0.0f, 5.0f);
	testEffectParticleSystem->SetFrames(2, 2, 0, 0, 4);
	testEffectParticleSystem->Compose(device, commandList, globalConstBufferId);
	
	auto effectAtlasId = resourceManager.CreateTexture("Resources\\Textures\\EffectAtlas.dds");

	testEffectMaterial = std::shared_ptr<Material>(new Material());
	testEffectMaterial->SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT);
	testEffectMaterial->SetVertexShader({ particleStandardVS, sizeof(particleStandardVS) });
	testEffectMaterial->SetGeometryShader({ particleStandardGS, sizeof(particleStandardGS) });
	testEffectMaterial->SetPixelShader({ particleStandardPS, sizeof(particleStandardPS) });
	testEffectMaterial->SetBlendMode(true, D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_ONE);
	testEffectMaterial->SetDepthTest(false);
	testEffectMaterial->SetDepthStencilFormat(32);
	testEffectMaterial->AssignBuffer(0, testEffectParticleSystem->GetParticleBufferId());
	testEffectMaterial->AssignTexture(commandList, 1, effectAtlasId, true);
	testEffectMaterial->SetSampler(0, D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 1);
	testEffectMaterial->AssignConstantBuffer(0, globalConstBufferId);
	testEffectMaterial->AssignConstantBuffer(1, testEffectParticleSystem->GetParticleSystemConstBufferId());
	testEffectMaterial->SetRenderTargetFormat(0, DXGI_FORMAT_R16G16B16A16_FLOAT);
	testEffectMaterial->SetRenderTargetFormat(1, DXGI_FORMAT_R8G8B8A8_SNORM);
	testEffectMaterial->Compose(device);

	testEffect = std::shared_ptr<GraphicObject>(new GraphicObject());
	testEffect->SetRenderingLayer(RenderingLayer::RENDERING_LAYER_EFFECT);
	testEffect->AssignRenderableEntity(testEffectParticleSystem.get());
	testEffect->AssignMaterial(testEffectMaterial.get());

	//auto testClothMesh = std::shared_ptr<Mesh>(new Mesh("Resources\\Meshes\\Cube.obj", true, false, false));
	//testClothCloth = std::shared_ptr<Cloth>(new Cloth(commandList, testClothMesh.get()));

	auto testSpriteTextureId = resourceManager.CreateTexture("Resources\\Textures\\TestSprite.dds");

	SpriteUI testUISprite(200, 200, { 10.0f, 10.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, testSpriteTextureId, false, {});
	
	testUISpriteMaterial = std::shared_ptr<Material>(new Material());
	testUISpriteMaterial->SetVertexFormat(VertexFormat::POSITION);
	testUISpriteMaterial->SetVertexShader({ uiSpriteStandardVS, sizeof(uiSpriteStandardVS) });
	testUISpriteMaterial->SetPixelShader({ uiSpriteStandardPS, sizeof(uiSpriteStandardPS) });
	testUISpriteMaterial->AssignConstantBuffer(0, testUISprite.GetConstantBufferId());
	testUISpriteMaterial->AssignTexture(commandList, 0, testSpriteTextureId, true);
	testUISpriteMaterial->SetSampler(0, D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 1);
	testUISpriteMaterial->SetBlendMode(true, D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA);
	testUISpriteMaterial->SetDepthTest(false);
	testUISpriteMaterial->SetRenderTargetFormat(0, DXGI_FORMAT_R8G8B8A8_UNORM);
	testUISpriteMaterial->SetDepthStencilFormat(32);
	testUISpriteMaterial->Compose(device);

	testUISprite.SetMaterial(testUISpriteMaterial.get());

	currentScene->GetUISystem()->AddSprite(testUISprite);

	auto testFontTextureId = resourceManager.CreateTexture("Resources\\Textures\\PixelArtFont.dds");

	testUIFont = std::shared_ptr<Font>(new Font(testFontTextureId, 16, 16, ' '));
	
	TextUI testUIText(device, 200, 500, { 1.0f, 1.0f }, 12.0f, 1.0f, 12.0f, 100, { 1.0f, 1.0f, 1.0f, 1.0f }, testUIFont.get());

	testUITextMaterial = std::shared_ptr<Material>(new Material());
	testUITextMaterial->AddCustomInputLayoutElement("POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1);
	testUITextMaterial->AddCustomInputLayoutElement("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1);
	testUITextMaterial->AddCustomInputLayoutElement("TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1);
	testUITextMaterial->AddCustomInputLayoutElement("TEXCOORD", 2, DXGI_FORMAT_R32G32_FLOAT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1);
	testUITextMaterial->SetVertexShader({ uiTextStandardVS, sizeof(uiTextStandardVS) });
	testUITextMaterial->SetPixelShader({ uiTextStandardPS, sizeof(uiTextStandardPS) });
	testUITextMaterial->AssignConstantBuffer(0, testUIText.GetConstantBufferId());
	testUITextMaterial->AssignTexture(commandList, 0, testFontTextureId, true);
	testUITextMaterial->SetSampler(0, D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 1);
	testUITextMaterial->SetBlendMode(true, D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA);
	testUITextMaterial->SetDepthTest(false);
	testUITextMaterial->SetRenderTargetFormat(0, DXGI_FORMAT_R8G8B8A8_UNORM);
	testUITextMaterial->SetDepthStencilFormat(32);
	testUITextMaterial->Compose(device);

	testUIText.SetMaterial(testUITextMaterial.get());
	testUIText.SetString(commandList, "Ничего себе!\nЗдесь теперь тоже есть вывод текста!");

	currentScene->GetUISystem()->AddText(testUIText);

	currentScene->SetMainCamera(camera.get());
	currentScene->EmplaceGraphicObject(goldenFrame.get(), false);
	//currentScene->EmplaceGraphicObject(cubeSecond.get(), false);
	currentScene->EmplaceGraphicObject(testEffect.get(), false);
}

void Graphics::SceneManager::ExecuteScripts(ID3D12GraphicsCommandList* commandList, size_t mouseX, size_t mouseY)
{
	currentScene->ExecuteScripts(commandList, mouseX, mouseY);
}

void Graphics::SceneManager::DrawCurrentScene(ID3D12GraphicsCommandList* commandList) const
{
	if (currentScene == nullptr)
		return;

	cameraShift += 0.005f;

	camera->Move(float3(std::cos(cameraShift) * 15.0f, 7.0f, std::sin(cameraShift) * 15.0f));
	camera->LookAt(float3(0.0f, 5.0f, 0.0f));
	//camera->Move(float3(std::cos(cameraShift) * 4.0f, 2.0f, -6.0f));
	camera->Update();
	
	globalConstBuffer.view = camera->GetView();
	globalConstBuffer.viewProjection = camera->GetViewProjection();
	globalConstBuffer.invView = camera->GetInvView();
	globalConstBuffer.invViewProjection = camera->GetInvProjection();
	globalConstBuffer.cameraPosition = camera->GetPosition();
	globalConstBuffer.elapsedTime = 1.0f / Graphics::GraphicsSettings::GetFramesPerSecond();
	globalConstBuffer.randomValues = { Random01(randomEngine.get()), Random01(randomEngine.get()), Random01(randomEngine.get()), Random01(randomEngine.get()) };

	resourceManager.UpdateConstantBuffer(globalConstBufferId, &globalConstBuffer, sizeof(globalConstBuffer));

	float3 translation = float3(1.0f, 0.0f, 1.0f);
	float3 rotationOrigin = float3(0.0f, 0.0f, 0.0f);
	floatN rotation = XMQuaternionRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
	float3 scale = float3(1.0f, 1.0f, 1.0f);
	
	goldenFrameConstBuffer.world = XMMatrixAffineTransformation(XMLoadFloat3(&scale), XMLoadFloat3(&rotationOrigin), rotation, XMLoadFloat3(&translation));
	goldenFrameConstBuffer.wvp = XMMatrixMultiply(goldenFrameConstBuffer.world, camera->GetViewProjection());// XMMatrixMultiply(goldenFrameConstBuffer.world, camera->GetViewProjection());
	
	/*float3 translation2 = float3(0.0f, 0.0f, 30.0f);
	float3 rotationOrigin2 = float3(0.0f, 0.0f, 0.0f);
	floatN rotation2 = XMQuaternionRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
	float3 scale2 = float3(20.0f, 20.0f, 20.0f);

	cubeSecondConstBuffer.world = XMMatrixAffineTransformation(XMLoadFloat3(&scale2), XMLoadFloat3(&rotationOrigin2), rotation2, XMLoadFloat3(&translation2));
	cubeSecondConstBuffer.wvp = XMMatrixMultiply(cubeSecondConstBuffer.world, camera->GetViewProjection());*/
	
	goldenFrameMaterial->UpdateConstantBuffer(goldenFrameConstBufferId, &goldenFrameConstBuffer, sizeof(goldenFrameConstBuffer));
	//cubeSecondMaterial->UpdateConstantBuffer(cubeSecondConstBufferId, &cubeSecondConstBuffer, sizeof(cubeSecondConstBuffer));

	currentScene->Draw(commandList);
}

void Graphics::SceneManager::DrawUI(ID3D12GraphicsCommandList* commandList) const
{
	currentScene->DrawUI(commandList);
}

Graphics::SceneManager::SceneManager()
	: currentScene(nullptr), immutableGlobalConstBuffer{}, globalConstBuffer{}, goldenFrameConstBuffer{}, cameraShift{}
{
	std::random_device randomDevice;

	randomEngine = std::shared_ptr<std::default_random_engine>(new std::default_random_engine(randomDevice()));
}

Graphics::SceneManager::~SceneManager()
{

}
