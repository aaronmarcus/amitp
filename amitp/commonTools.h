#pragma once

#ifndef COMMON_TOOLS_H
#define COMMON_TOOLS_H

#include <cstddef>

//TODO refactor CHECK_HR to CheckHR
#define CHECK_HR(hr, msg) if (hr != S_OK) { printf(msg); printf(" Error: %.2X.\n", hr); goto done; }

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

#endif


