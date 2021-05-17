#include "SceneManager.h"
#include "Resources/Shaders/MeshStandard.vsh.h"
#include "Resources/Shaders/MeshStandard.psh.h"

Graphics::SceneManager& Graphics::SceneManager::GetInstance()
{
	static SceneManager sceneManager;

	return sceneManager;
}

Graphics::Scene* Graphics::SceneManager::CreateNewScene()
{
	scenes.push_back(Scene());

	return &scenes.back();
}

void Graphics::SceneManager::InitializeTestScene(ID3D12Device* device)
{
	const float fovAngleY = XM_PI / 4.0f;
	const float aspectRatio = GraphicsSettings::GetResolutionX() / static_cast<float>(GraphicsSettings::GetResolutionY());
	const float nearZ = 0.01f;
	const float farZ = 1024.0f;

	camera = std::make_shared<Camera>(fovAngleY, aspectRatio, nearZ, farZ);

	camera->Move({ -5.0f, 2.0f, -5.0f });
	camera->LookAt({ 0.0f, 0.0f, 0.0f });

	camera->Update();

	currentScene = CreateNewScene();

	cubeMesh = std::make_shared<Mesh>("Resources\\Meshes\\Cube.obj");
	cubeMaterial = std::make_shared<Material>();

	cubeMaterial->SetVertexFormat(cubeMesh->GetVertexFormat());
	cubeMaterial->SetVertexShader({ meshStandardVertexShader, sizeof(meshStandardVertexShader) });
	cubeMaterial->SetPixelShader({ meshStandardPixelShader, sizeof(meshStandardPixelShader) });
	cubeMaterial->SetDepthTest(true);
	cubeMaterial->SetDepthStencilFormat(32);
	cubeMaterial->SetCullMode(D3D12_CULL_MODE_NONE);
	cubeMaterial->SetRenderTargetFormat(0, DXGI_FORMAT_R16G16B16A16_FLOAT);
	cubeMaterial->SetRenderTargetFormat(1, DXGI_FORMAT_R8G8B8A8_SNORM);

	cubeConstBuffer.world = XMMatrixIdentity();
	cubeConstBuffer.wvp = camera->GetViewProjection();

	cubeConstBufferId = cubeMaterial->SetConstantBuffer(0, &cubeConstBuffer, sizeof(cubeConstBuffer));

	cubeMaterial->Compose(device);

	cube = std::make_shared<GraphicObject>();
	cube->AssignMesh(cubeMesh.get());
	cube->AssignMaterial(cubeMaterial.get());

	cubeSecondMaterial = std::make_shared<Material>();

	cubeSecondMaterial->SetVertexFormat(cubeMesh->GetVertexFormat());
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
	cubeSecond->AssignMesh(cubeMesh.get());
	cubeSecond->AssignMaterial(cubeSecondMaterial.get());

	currentScene->SetMainCamera(camera.get());
	currentScene->EmplaceGraphicObject(cube.get(), false);
	currentScene->EmplaceGraphicObject(cubeSecond.get(), false);
}

void Graphics::SceneManager::DrawCurrentScene(ID3D12GraphicsCommandList* commandList)
{
	if (currentScene == nullptr)
		return;

	cameraShift += 0.005f;

	camera->Move(float3(std::cos(cameraShift) * 5.0f, 2.0f, std::sin(cameraShift) * 5.0f));
	camera->Update();

	cubeConstBuffer.wvp = camera->GetViewProjection();

	float3 translation = float3(-std::cos(cameraShift) * 10.0f, -5.0f, -std::sin(cameraShift) * 10.0f);
	float3 rotationOrigin = float3(0.0f, 0.0f, 0.0f);
	floatN rotation = XMQuaternionRotationRollPitchYaw(0.0f, XM_PI / 4.0f, 0.0f);
	float3 scale = float3(5.0f, 5.0f, 5.0f);

	cubeSecondConstBuffer.world = XMMatrixAffineTransformation(XMLoadFloat3(&scale), XMLoadFloat3(&rotationOrigin), rotation, XMLoadFloat3(&translation));
	cubeSecondConstBuffer.wvp = XMMatrixMultiply(cubeSecondConstBuffer.world, camera->GetViewProjection());

	cubeMaterial->UpdateConstantBuffer(cubeConstBufferId, &cubeConstBuffer, sizeof(cubeConstBuffer));
	cubeSecondMaterial->UpdateConstantBuffer(cubeSecondConstBufferId, &cubeSecondConstBuffer, sizeof(cubeSecondConstBuffer));

	currentScene->Draw(commandList);
}

Graphics::SceneManager::SceneManager()
	: currentScene(nullptr), cubeConstBuffer{}, cameraShift{}
{
	
}

Graphics::SceneManager::~SceneManager()
{

}
