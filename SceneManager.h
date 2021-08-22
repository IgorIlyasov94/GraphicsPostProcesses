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

		void ExecuteScripts(ID3D12GraphicsCommandList* commandList, size_t mouseX, size_t mouseY);
		void DrawCurrentScene(ID3D12GraphicsCommandList* commandList) const;
		void DrawUI(ID3D12GraphicsCommandList* commandList) const;

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
		std::shared_ptr<Material> goldenFrameMaterial;
		std::shared_ptr<Mesh> goldenFrameMesh;
		
		std::shared_ptr<GraphicObject> testEffect;
		std::shared_ptr<Material> testEffectMaterial;
		std::shared_ptr<ParticleSystem> testEffectParticleSystem;

		std::shared_ptr<GraphicObject> testCloth;
		std::shared_ptr<Material> testClothMaterial;
		std::shared_ptr<Cloth> testClothCloth;

		std::shared_ptr<Material> testUISpriteMaterial;
		std::shared_ptr<Material> testUITextMaterial;

		std::shared_ptr<Font> testUIFont;

		ConstantBufferId goldenFrameConstBufferId;
		ConstantBufferId clothConstBufferId;

		ConstantBufferId immutableGlobalConstBufferId;
		ConstantBufferId globalConstBufferId;

		mutable float cameraShift;

		struct ImmutableGlobalConstBuffer
		{
			float4x4 projection;
			float4x4 invProjection;
			float zNear;
			float zFar;
			float2 zLinearizeCoeff;
		};

		struct GlobalConstBuffer
		{
			float4x4 view;
			float4x4 invView;
			float4x4 viewProjection;
			float4x4 invViewProjection;
			float4 randomValues;
			float3 cameraPosition;
			float elapsedTime;
			float previousElapsedTime;
			float3 padding;
		};

		struct StandardMeshConstBuffer
		{
			float4x4 world;
			float4x4 wvp;
		};

		mutable StandardMeshConstBuffer goldenFrameConstBuffer;
		mutable StandardMeshConstBuffer clothConstBuffer;

		ImmutableGlobalConstBuffer immutableGlobalConstBuffer;
		mutable GlobalConstBuffer globalConstBuffer;

		std::shared_ptr<Camera> camera;

		std::shared_ptr<std::default_random_engine> randomEngine;

		ResourceManager& resourceManager = ResourceManager::GetInstance();
	};
}
