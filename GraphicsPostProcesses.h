#pragma once

#include "stdafx.h"
#include "GraphicsRendererDirectX12.h"

class GraphicsWinApplication
{
public:
	static GraphicsWinApplication& getInstance();

	int run(const HINSTANCE& instance, const LPSTR& cmdLine, const int& cmdShow);
private:
	GraphicsWinApplication();
	~GraphicsWinApplication() {};

	GraphicsWinApplication(const GraphicsWinApplication&) = delete;
	GraphicsWinApplication(GraphicsWinApplication&&) = delete;
	GraphicsWinApplication& operator=(const GraphicsWinApplication&) = delete;
	GraphicsWinApplication& operator=(GraphicsWinApplication&&) = delete;

	static LRESULT CALLBACK windowProc(HWND windowHandler, UINT message, WPARAM wParam, LPARAM lParam);

	GraphicsRendererDirectX12& renderer = GraphicsRendererDirectX12::getInstance();
};