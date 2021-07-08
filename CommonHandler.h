#pragma once

#include "RendererDirectX12.h"
#include "SceneManager.h"

namespace Graphics
{
	class CommonHandler
	{
	public:
		static CommonHandler& GetInstance();
		
		void Initialize(HWND& windowHandler);
		void Update();
		void Stop();

		void OnKeyDown(uint8_t key);
		void OnKeyUp(uint8_t key);

		void OnSetFocus();
		void OnLostFocus();

	private:
		CommonHandler();
		~CommonHandler() {}

		CommonHandler(const CommonHandler&) = delete;
		CommonHandler(CommonHandler&&) = delete;
		CommonHandler& operator=(const CommonHandler&) = delete;
		CommonHandler& operator=(CommonHandler&&) = delete;

		HWND windowHandlerPtr;

		size_t mouseX;
		size_t mouseY;

		bool inFocus;

		RendererDirectX12& renderer = RendererDirectX12::GetInstance();
		SceneManager& sceneManager = SceneManager::GetInstance();
	};
}
