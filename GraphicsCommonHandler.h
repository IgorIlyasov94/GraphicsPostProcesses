#pragma once

#include "GraphicsRendererDirectX12.h"

class GraphicsCommonHandler
{
public:
	static GraphicsCommonHandler& GetInstance();
	GraphicsRendererDirectX12& getRenderer();

	void Initialize(HWND& windowHandler);
	void Update();
	void Stop();

	void OnKeyDown(uint8_t key);
	void OnKeyUp(uint8_t key);

private:
	GraphicsCommonHandler();
	~GraphicsCommonHandler() {}

	GraphicsCommonHandler(const GraphicsCommonHandler&) = delete;
	GraphicsCommonHandler(GraphicsCommonHandler&&) = delete;
	GraphicsCommonHandler& operator=(const GraphicsCommonHandler&) = delete;
	GraphicsCommonHandler& operator=(GraphicsCommonHandler&&) = delete;

	GraphicsRendererDirectX12& renderer = GraphicsRendererDirectX12::GetInstance();
};
