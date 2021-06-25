#include "SceneManager.h"
#include "Resources/Shaders/MeshStandardVS.hlsl.h"
#include "Resources/Shaders/MeshStandardPS.hlsl.h"

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
	
	goldenFrameMesh = std::shared_ptr<Mesh>(new Mesh("Resources\\Meshes\\Cube.obj", true, false, false));
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
	goldenFrame->AssignMesh(goldenFrameMesh.get());
	goldenFrame->AssignMaterial(goldenFrameMaterial.get());

	/*cubeMeshSecond = std::make_shared<Mesh>("Resources\\Meshes\\Cube.obj", true, false, false);

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

	cubeSecondMaterial->AssignBuffer(0, currentScene->GetLightingSystem()->GetLightBufferId());
	cubeSecondMaterial->AssignTexture(1, currentScene->GetLightingSystem()->GetLightClusterId());
	
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

	camera->Move(float3(std::cos(cameraShift) * 15.0f, 7.0f, std::sin(cameraShift) * 15.0f));
	camera->LookAt(float3(0.0f, 3.0f, 0.0f));
	//camera->Move(float3(std::cos(cameraShift) * 4.0f, 2.0f, -6.0f));
	camera->Update();
	
	globalConstBuffer.view = camera->GetView();
	globalConstBuffer.viewProjection = camera->GetViewProjection();
	globalConstBuffer.invView = camera->GetInvView();
	globalConstBuffer.invViewProjection = camera->GetInvProjection();
	globalConstBuffer.cameraPosition = camera->GetPosition();

	resourceManager.UpdateConstantBuffer(globalConstBufferId, &globalConstBuffer, sizeof(globalConstBuffer));

	float3 translation = float3(1.0f, 0.0f, 1.0f);
	float3 rotationOrigin = float3(0.0f, 0.0f, 0.0f);
	floatN rotation = XMQuaternionRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
	float3 scale = float3(2.0f, 10.0f, 2.0f);
	
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

Graphics::SceneManager::SceneManager()
	: currentScene(nullptr), immutableGlobalConstBuffer{}, globalConstBuffer{}, goldenFrameConstBuffer{}, cameraShift{}
{
	
}

Graphics::SceneManager::~SceneManager()
{

}
