#pragma once

#include "stdafx.h"
#include "GraphicsCommonHandler.h"

class GraphicsWinApplication
{
public:
	static GraphicsWinApplication& GetInstance();

	int Run(const HINSTANCE& instance, const LPSTR& cmdLine, const int& cmdShow);
private:
	GraphicsWinApplication();
	~GraphicsWinApplication() {}

	GraphicsWinApplication(const GraphicsWinApplication&) = delete;
	GraphicsWinApplication(GraphicsWinApplication&&) = delete;
	GraphicsWinApplication& operator=(const GraphicsWinApplication&) = delete;
	GraphicsWinApplication& operator=(GraphicsWinApplication&&) = delete;

	static LRESULT CALLBACK WindowProc(HWND windowHandler, UINT message, WPARAM wParam, LPARAM lParam);

	GraphicsCommonHandler& commonHandler = GraphicsCommonHandler::GetInstance();
};