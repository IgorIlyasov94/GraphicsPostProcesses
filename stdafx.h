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

#include <d3dcompiler.h>

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
        errorMessage << "HRESULT: ";

        switch (hResult)
        {
        case E_ABORT:
            errorMessage << "E_ABORT (Operation aborted)";
            break;
        case E_ACCESSDENIED:
            errorMessage << "E_ACCESSDENIED (General access denied error)";
            break;
        case E_FAIL:
            errorMessage << "E_FAIL (Unspecified failure)";
            break;
        case E_HANDLE:
            errorMessage << "E_HANDLE (Handle that is not valid)";
            break;
        case E_INVALIDARG:
            errorMessage << "E_INVALIDARG (One or more arguments are not valid)";
            break;
        case E_NOINTERFACE:
            errorMessage << "E_NOINTERFACE (No such interface supported)";
            break;
        case E_NOTIMPL:
            errorMessage << "E_NOTIMPL (Not implemented)";
            break;
        case E_OUTOFMEMORY:
            errorMessage << "E_OUTOFMEMORY (Failed to allocate necessary memory)";
            break;
        case E_POINTER:
            errorMessage << "E_POINTER (Pointer that is not valid)";
            break;
        case E_UNEXPECTED:
            errorMessage << "E_UNEXPECTED (Unexpected failure)";
            break;
        default:
            errorMessage << "0x" << std::hex << hResult;
        }

        throw std::runtime_error(errorMessage.str());
    }
}