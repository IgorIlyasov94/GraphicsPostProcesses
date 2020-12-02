#include "GraphicsCommonHandler.h"

GraphicsCommonHandler::GraphicsCommonHandler()
{

}

GraphicsCommonHandler& GraphicsCommonHandler::getInstance()
{
	static GraphicsCommonHandler instance;

	return instance;
}

GraphicsRendererDirectX12& GraphicsCommonHandler::getRenderer()
{
	return renderer;
}

void GraphicsCommonHandler::initialize(HWND& windowHandler)
{
	renderer.initialize(windowHandler);
}

void GraphicsCommonHandler::update()
{
	renderer.frameRender();
}

void GraphicsCommonHandler::stop()
{
	renderer.gpuRelease();
}

void GraphicsCommonHandler::onKeyDown(uint8_t key)
{
	switch (key)
	{
	case VK_ESCAPE:
	{
		ExitProcess(0);

		break;
	}
	}
}

void GraphicsCommonHandler::onKeyUp(uint8_t key)
{

}