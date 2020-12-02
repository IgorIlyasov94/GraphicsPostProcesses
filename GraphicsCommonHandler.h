#pragma once

#include "GraphicsRendererDirectX12.h"

class GraphicsCommonHandler
{
public:
	static GraphicsCommonHandler& getInstance();
	GraphicsRendererDirectX12& getRenderer();

	void initialize(HWND& windowHandler);
	void update();
	void stop();

	void onKeyDown(uint8_t key);
	void onKeyUp(uint8_t key);

private:
	GraphicsCommonHandler();
	~GraphicsCommonHandler() {};

	GraphicsCommonHandler(const GraphicsCommonHandler&) = delete;
	GraphicsCommonHandler(GraphicsCommonHandler&&) = delete;
	GraphicsCommonHandler& operator=(const GraphicsCommonHandler&) = delete;
	GraphicsCommonHandler& operator=(GraphicsCommonHandler&&) = delete;

	GraphicsRendererDirectX12& renderer = GraphicsRendererDirectX12::getInstance();
};