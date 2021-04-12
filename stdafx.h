#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include <d3d11.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#include <DirectXMath.h>

#include <wrl/client.h>

#include <cstdint>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <exception>
#include <stdexcept>
#include <vector>
#include <array>
#include <deque>
#include <list>
#include <mutex>
#include <set>
#include <algorithm>
#include <regex>
#include <set>

inline void ThrowIfFailed(HRESULT hResult, const char* errorMessage)
{
    if (FAILED(hResult))
    {
        std::stringstream fullMessage;
        fullMessage << errorMessage << "\nHRESULT: " << "0x" << std::hex << hResult;
        
        throw std::runtime_error(fullMessage.str());
    }
}