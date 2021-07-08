#include "CommonHandler.h"

Graphics::CommonHandler::CommonHandler()
	: windowHandlerPtr(nullptr), mouseX(0), mouseY(0), inFocus(true)
{

}

Graphics::CommonHandler& Graphics::CommonHandler::GetInstance()
{
	static CommonHandler instance;

	return instance;
}

void Graphics::CommonHandler::Initialize(HWND& windowHandler)
{
	windowHandlerPtr = windowHandler;

	renderer.Initialize(windowHandler);
}

void Graphics::CommonHandler::Update()
{
	renderer.FrameStart();

	POINT cursorPos;
	if (GetCursorPos(&cursorPos))
	{
		if (ScreenToClient(windowHandlerPtr, &cursorPos))
		{
			mouseX = cursorPos.x;
			mouseY = cursorPos.y;
		}
	}

	sceneManager.ExecuteScripts(renderer.GetCommandList(), mouseX, mouseY);

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
