#pragma once

#include <Windows.h>

//TODO refactor CHECK_HR to CheckHR
//TODO change this for an inline function
#define CHECK_HR(hr, msg) if (hr != S_OK) { OutputDebugStringW(msg); goto done; }


//TODO refactor SAFE_RELEASE too SafeRelease
template <class T> void SAFE_RELEASE(T * *ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

template <class T> inline void SAFE_RELEASE(T * &pT)
{
    if (pT != NULL)
    {
        pT->Release();
        pT = NULL;
    }
}

