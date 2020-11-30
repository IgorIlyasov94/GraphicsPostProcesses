#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <d3d12.h>
#include <dxgi1_6.h>

#include <wrl/client.h>

#include <cstdint>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <exception>
#include <stdexcept>

inline void ThrowIfFailed(HRESULT hResult)
{
    if (FAILED(hResult))
    {
        std::stringstream errorMessage;
        errorMessage << "HRESULT: 0x" << std::hex << hResult;

        throw std::runtime_error(errorMessage.str());
    }
}