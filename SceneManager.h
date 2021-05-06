#pragma once

#include "Scene.h"

namespace Graphics
{
	class SceneManager
	{
	public:
		static SceneManager& GetInstance();

		Scene* CreateNewScene();

		void InitializeTestScene(ID3D12Device* device);

		void DrawCurrentScene(ID3D12GraphicsCommandList* commandList);

	private:
		SceneManager();
		~SceneManager();

		SceneManager(const SceneManager&) = delete;
		SceneManager(SceneManager&&) = delete;
		SceneManager& operator=(const SceneManager&) = delete;
		SceneManager& operator=(SceneManager&&) = delete;

		std::list<Scene> scenes;

		Scene* currentScene;

		//temp

		std::shared_ptr<GraphicObject> cube;
		std::shared_ptr<Material> cubeMaterial;
		std::shared_ptr<Mesh> cubeMesh;

		ConstantBufferId cubeConstBufferId;

		float cameraShift;

		struct CubeConstBuffer
		{
			float4x4 wvp;
		} cubeConstBuffer;

		std::shared_ptr<Camera> camera;
	};
}
