#include "GraphicsCommonHandler.h"

GraphicsCommonHandler::GraphicsCommonHandler()
{

}

GraphicsCommonHandler& GraphicsCommonHandler::GetInstance()
{
	static GraphicsCommonHandler instance;

	return instance;
}

GraphicsRendererDirectX12& GraphicsCommonHandler::getRenderer()
{
	return renderer;
}

void GraphicsCommonHandler::Initialize(HWND& windowHandler)
{
	renderer.Initialize(windowHandler);
}

void GraphicsCommonHandler::Update()
{
	renderer.FrameRender();
}

void GraphicsCommonHandler::Stop()
{
	renderer.GpuRelease();
}

void GraphicsCommonHandler::OnKeyDown(uint8_t key)
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

void GraphicsCommonHandler::OnKeyUp(uint8_t key)
{

}