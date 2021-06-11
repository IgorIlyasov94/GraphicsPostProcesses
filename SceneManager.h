#pragma once

#include "Scene.h"
#include "LightingSystem.h"

namespace Graphics
{
	class SceneManager
	{
	public:
		static SceneManager& GetInstance();

		Scene* CreateNewScene();

		void InitializeTestScene(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);

		void ExecuteScripts(ID3D12GraphicsCommandList* commandList);
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

		std::shared_ptr<GraphicObject> goldenFrame;
		std::shared_ptr<ComputeObject> goldenFrameCompute;
		std::shared_ptr<Material> goldenFrameMaterial;
		std::shared_ptr<Mesh> goldenFrameMesh;
		
		/*std::shared_ptr<GraphicObject> cubeSecond;
		std::shared_ptr<Material> cubeSecondMaterial;
		std::shared_ptr<Mesh> cubeMeshSecond;*/

		ConstantBufferId goldenFrameConstBufferId;
		//ConstantBufferId cubeSecondConstBufferId;

		float cameraShift;

		struct StandardMeshConstBuffer
		{
			float4x4 world;
			float4x4 wvp;
		};

		StandardMeshConstBuffer goldenFrameConstBuffer;
		//StandardMeshConstBuffer cubeSecondConstBuffer;

		std::shared_ptr<Camera> camera;

		ResourceManager& resourceManager = ResourceManager::GetInstance();
	};
}
