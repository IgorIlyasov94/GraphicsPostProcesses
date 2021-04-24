#pragma once

#include "Scene.h"

namespace Graphics
{
	class SceneManager
	{
	public:
		static SceneManager& GetInstance();

		Scene* CreateNewScene();

		void DrawCurrentScene(ID3D12GraphicsCommandList* commandList) const;

	private:
		SceneManager();
		~SceneManager();

		SceneManager(const SceneManager&) = delete;
		SceneManager(SceneManager&&) = delete;
		SceneManager& operator=(const SceneManager&) = delete;
		SceneManager& operator=(SceneManager&&) = delete;

		std::list<Scene> scenes;

		Scene* currentScene;
	};
}
