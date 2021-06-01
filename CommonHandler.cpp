#include "CommonHandler.h"

Graphics::CommonHandler::CommonHandler()
	: inFocus(true)
{

}

Graphics::CommonHandler& Graphics::CommonHandler::GetInstance()
{
	static CommonHandler instance;

	return instance;
}

void Graphics::CommonHandler::Initialize(HWND& windowHandler)
{
	renderer.Initialize(windowHandler);
}

void Graphics::CommonHandler::Update()
{
	renderer.FrameStart();

	sceneManager.ExecuteScripts(renderer.GetCommandList());

	renderer.FrameRender();
}

void Graphics::CommonHandler::Stop()
{
	renderer.GpuRelease();
}

void Graphics::CommonHandler::OnKeyDown(uint8_t key)
{
	switch (key)
	{
	case VK_ESCAPE:
	{
		PostQuitMessage(0);

		break;
	}
	}
}

void Graphics::CommonHandler::OnKeyUp(uint8_t key)
{

}

void Graphics::CommonHandler::OnSetFocus()
{
	if (!inFocus)
	{
		renderer.OnSetFocus();

		inFocus = true;
	}
}

void Graphics::CommonHandler::OnLostFocus()
{
	if (inFocus)
	{
		renderer.OnLostFocus();

		inFocus = false;
	}
}
