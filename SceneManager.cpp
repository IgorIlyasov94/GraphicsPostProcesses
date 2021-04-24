#include "SceneManager.h"

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

void Graphics::SceneManager::DrawCurrentScene(ID3D12GraphicsCommandList* commandList) const
{
	if (currentScene == nullptr)
		return;

	currentScene->Draw(commandList);
}

Graphics::SceneManager::SceneManager()
	: currentScene(nullptr)
{

}

Graphics::SceneManager::~SceneManager()
{

}
