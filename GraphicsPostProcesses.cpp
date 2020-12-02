#include "GraphicsPostProcesses.h"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow)
{
	GraphicsWinApplication& winApp = GraphicsWinApplication::getInstance();

	return winApp.run(instance, cmdLine, cmdShow);
}

GraphicsWinApplication::GraphicsWinApplication()
{
	
}

LRESULT GraphicsWinApplication::windowProc(HWND windowHandler, UINT message, WPARAM wParam, LPARAM lParam)
{
	GraphicsCommonHandler* transmittedCommonHandler = reinterpret_cast<GraphicsCommonHandler*>(GetWindowLongPtr(windowHandler, GWLP_USERDATA));

	switch (message)
	{
	case WM_CREATE:
	{
		LPCREATESTRUCT createStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		SetWindowLongPtr(windowHandler, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(createStruct->lpCreateParams));

		return 0;
	}
	case WM_KEYDOWN:
	{
		auto keyData = static_cast<uint8_t>(wParam);

		transmittedCommonHandler->onKeyDown(keyData);

		return 0;
	}
	case WM_KEYUP:
	{
		auto keyData = static_cast<uint8_t>(wParam);

		transmittedCommonHandler->onKeyUp(keyData);

		return 0;
	}
	case WM_PAINT:
	{
		transmittedCommonHandler->update();

		return 0;
	}
	case WM_DESTROY:
	{
		PostQuitMessage(0);

		return 0;
	}
	case WM_CLOSE:
	{
		PostQuitMessage(0);

		return 0;
	}
	default:
	{
		return DefWindowProc(windowHandler, message, wParam, lParam);
	}
	}
}

GraphicsWinApplication& GraphicsWinApplication::getInstance()
{
	static GraphicsWinApplication instance;

	return instance;
}

int GraphicsWinApplication::run(const HINSTANCE& instance, const LPSTR& cmdLine, const int& cmdShow)
{
	try
	{
		WNDCLASSEX windowClass{};

		auto windowClassName = L"GraphicsPostProcess";
		auto windowTitleName = L"GraphicsPostProcess";

		windowClass.cbSize = sizeof(WNDCLASSEX);
		windowClass.style = CS_HREDRAW | CS_VREDRAW;
		windowClass.lpfnWndProc = windowProc;
		windowClass.hInstance = instance;
		windowClass.hIcon = LoadIcon(instance, IDI_APPLICATION);
		windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
		windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		windowClass.lpszClassName = windowClassName;
		windowClass.hIconSm = LoadIcon(windowClass.hInstance, IDI_APPLICATION);

		RegisterClassExW(&windowClass);

		auto windowHandler = CreateWindowExW(WS_EX_APPWINDOW, windowClassName, windowTitleName, WS_OVERLAPPEDWINDOW,
			0, 0, commonHandler.getRenderer().getResolutionX(), commonHandler.getRenderer().getResolutionY(), nullptr, nullptr, instance, &commonHandler);

		ShowWindow(windowHandler, cmdShow);

		MSG message{};

		commonHandler.initialize(windowHandler);

		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE) != WM_QUIT)
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		commonHandler.stop();

		return static_cast<int>(message.wParam);
	}
	catch (const std::exception& e)
	{
		OutputDebugStringW(L"Error: ");
		OutputDebugStringA(e.what());

		commonHandler.stop();

		return EXIT_FAILURE;
	}
}