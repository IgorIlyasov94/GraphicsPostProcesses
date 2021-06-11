#include "SceneManager.h"
#include "Resources/Shaders/MeshStandard.vsh.h"
#include "Resources/Shaders/MeshStandard.psh.h"

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
	camera->LookAt({ 0.0f, 3.0f, 0.0f });

	camera->Update();

	currentScene = CreateNewScene();

	currentScene->GetLightingSystem()->CreatePointLight(float3(0.0f, 0.0f, 0.0f), float3(1.0f, 0.5f, 0.7f), 2.0f, 1.0f, false);
	currentScene->GetLightingSystem()->CreatePointLight(float3(1.0f, 0.0f, 0.0f), float3(0.5f, 0.5f, 0.7f), 2.0f, 1.0f, false);
	currentScene->GetLightingSystem()->CreatePointLight(float3(0.0f, 0.0f, 1.0f), float3(0.0f, 1.0f, 0.7f), 2.0f, 1.0f, false);

	currentScene->GetLightingSystem()->ComposeLightBuffer(device, commandList);

	goldenFrameMesh = std::shared_ptr<Mesh>(new Mesh("Resources\\Meshes\\GoldenFrame.obj", true, false, true));
	goldenFrameMaterial = std::make_shared<Material>();

	goldenFrameMaterial->SetVertexFormat(goldenFrameMesh->GetVertexFormat());
	goldenFrameMaterial->SetVertexShader({ meshStandardVertexShader, sizeof(meshStandardVertexShader) });
	goldenFrameMaterial->SetPixelShader({ meshStandardPixelShader, sizeof(meshStandardPixelShader) });
	goldenFrameMaterial->SetDepthTest(true);
	goldenFrameMaterial->SetDepthStencilFormat(32);
	goldenFrameMaterial->SetCullMode(D3D12_CULL_MODE_BACK);
	goldenFrameMaterial->SetRenderTargetFormat(0, DXGI_FORMAT_R16G16B16A16_FLOAT);
	goldenFrameMaterial->SetRenderTargetFormat(1, DXGI_FORMAT_R8G8B8A8_SNORM);

	goldenFrameConstBuffer.world = XMMatrixIdentity();
	goldenFrameConstBuffer.wvp = camera->GetViewProjection();

	goldenFrameConstBufferId = goldenFrameMaterial->SetConstantBuffer(0, &goldenFrameConstBuffer, sizeof(goldenFrameConstBuffer));

	goldenFrameMaterial->AssignBuffer(0, currentScene->GetLightingSystem()->GetLightBufferId());
	goldenFrameMaterial->AssignTexture(1, currentScene->GetLightingSystem()->GetLightClusterId());

	goldenFrameMaterial->Compose(device);

	goldenFrame = std::make_shared<GraphicObject>();
	goldenFrame->AssignMesh(goldenFrameMesh.get());
	goldenFrame->AssignMaterial(goldenFrameMaterial.get());

	/*cubeMeshSecond = std::make_shared<Mesh>("Resources\\Meshes\\CubeTest.obj");

	cubeSecondMaterial = std::make_shared<Material>();

	cubeSecondMaterial->SetVertexFormat(cubeMeshSecond->GetVertexFormat());
	cubeSecondMaterial->SetVertexShader({ meshStandardVertexShader, sizeof(meshStandardVertexShader) });
	cubeSecondMaterial->SetPixelShader({ meshStandardPixelShader, sizeof(meshStandardPixelShader) });
	cubeSecondMaterial->SetDepthTest(true);
	cubeSecondMaterial->SetDepthStencilFormat(32);
	cubeSecondMaterial->SetCullMode(D3D12_CULL_MODE_NONE);
	cubeSecondMaterial->SetRenderTargetFormat(0, DXGI_FORMAT_R16G16B16A16_FLOAT);
	cubeSecondMaterial->SetRenderTargetFormat(1, DXGI_FORMAT_R8G8B8A8_SNORM);

	cubeSecondConstBuffer.world = XMMatrixIdentity();
	cubeSecondConstBuffer.wvp = camera->GetViewProjection();

	cubeSecondConstBufferId = cubeSecondMaterial->SetConstantBuffer(0, &cubeSecondConstBuffer, sizeof(cubeSecondConstBuffer));

	cubeSecondMaterial->Compose(device);

	cubeSecond = std::make_shared<GraphicObject>();
	cubeSecond->AssignMesh(cubeMeshSecond.get());
	cubeSecond->AssignMaterial(cubeSecondMaterial.get());*/

	currentScene->SetMainCamera(camera.get());
	currentScene->EmplaceGraphicObject(goldenFrame.get(), false);
	//currentScene->EmplaceGraphicObject(cubeSecond.get(), false);
}

void Graphics::SceneManager::ExecuteScripts(ID3D12GraphicsCommandList* commandList)
{
	currentScene->ExecuteScripts(commandList);
}

void Graphics::SceneManager::DrawCurrentScene(ID3D12GraphicsCommandList* commandList)
{
	if (currentScene == nullptr)
		return;

	cameraShift += 0.005f;

	//camera->Move(float3(std::cos(cameraShift) * 7.0f, std::sin(cameraShift) * 7.0f, -2.0f));
	//camera->LookAt(float3(std::cos(cameraShift) * 7.0f, std::sin(cameraShift) * 7.0f, 0.0f));
	camera->Move(float3(std::cos(cameraShift) * 13.0f, 3.0f, std::sin(cameraShift) * 13.0f));
	camera->Update();

	goldenFrameConstBuffer.wvp = camera->GetViewProjection();

	/*float3 translation = float3(-std::cos(cameraShift) * 10.0f, -5.0f, -std::sin(cameraShift) * 10.0f);
	float3 rotationOrigin = float3(0.0f, 0.0f, 0.0f);
	floatN rotation = XMQuaternionRotationRollPitchYaw(0.0f, XM_PI / 4.0f, 0.0f);
	float3 scale = float3(5.0f, 5.0f, 5.0f);

	cubeSecondConstBuffer.world = XMMatrixAffineTransformation(XMLoadFloat3(&scale), XMLoadFloat3(&rotationOrigin), rotation, XMLoadFloat3(&translation));
	cubeSecondConstBuffer.wvp = XMMatrixMultiply(cubeSecondConstBuffer.world, camera->GetViewProjection());*/

	goldenFrameMaterial->UpdateConstantBuffer(goldenFrameConstBufferId, &goldenFrameConstBuffer, sizeof(goldenFrameConstBuffer));
	//cubeSecondMaterial->UpdateConstantBuffer(cubeSecondConstBufferId, &cubeSecondConstBuffer, sizeof(cubeSecondConstBuffer));

	currentScene->Draw(commandList);
}

Graphics::SceneManager::SceneManager()
	: currentScene(nullptr), goldenFrameConstBuffer{}, cameraShift{}
{
	
}

Graphics::SceneManager::~SceneManager()
{

}
