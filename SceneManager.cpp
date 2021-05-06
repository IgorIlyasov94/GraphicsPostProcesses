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
	cubeMaterial->SetCullMode(D3D12_CULL_MODE_NONE);
	cubeMaterial->SetRenderTargetFormat(0, DXGI_FORMAT_R8G8B8A8_UNORM);

	cubeConstBuffer.wvp = camera->GetViewProjection();

	cubeMaterial->SetConstantBuffer(0, &cubeConstBuffer, sizeof(cubeConstBuffer));

	cubeMaterial->Compose(device);

	cube = std::make_shared<GraphicObject>();
	cube->AssignMesh(cubeMesh.get());
	cube->AssignMaterial(cubeMaterial.get());

	currentScene->SetMainCamera(camera.get());
	currentScene->EmplaceGraphicObject(cube.get(), false);
}

void Graphics::SceneManager::DrawCurrentScene(ID3D12GraphicsCommandList* commandList) const
{
	if (currentScene == nullptr)
		return;

	currentScene->Draw(commandList);
}

Graphics::SceneManager::SceneManager()
	: currentScene(nullptr), cubeConstBuffer{}
{
	
}

Graphics::SceneManager::~SceneManager()
{

}
