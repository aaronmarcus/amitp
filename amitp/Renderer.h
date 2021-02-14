#pragma once

#include <d3d9.h>
#include <dxva2api.h>

#include <evr.h>
#include <mfapi.h>
#include <Mferror.h>
#include <mfobjects.h>
#include <mfplay.h>
#include <mfreadwrite.h>

#include <stdio.h>
#include <tchar.h>
#include <iostream>
 
#include <windowsx.h>

#include "commonTools.h"

#pragma comment(lib, "mf.lib")
#pragma comment(lib, "evr.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfplay.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "Strmiids")
#pragma comment(lib, "wmcodecdspuuid.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "Dxva2.lib")


//define structure to describe the source format
struct sourceFormatDescriptor
{
	unsigned __int32 width;
	unsigned __int32 height;
	unsigned __int32 bitDepth;
};


class Renderer
{
	public:
		Renderer(unsigned __int32 m_width, unsigned __int32 m_height, unsigned __int32 m_bitDepth, HWND m_hwnd);
		HRESULT ShutdownRenderer();
		HRESULT SetVideoSource();
};

class MediaEventHandler : IMFAsyncCallback
{
	HRESULT STDMETHODCALLTYPE Invoke(IMFAsyncResult* pAsyncResult);
	HRESULT STDMETHODCALLTYPE GetParameters(
		DWORD* pdwFlags,
		DWORD* pdwQueue
	);
	HRESULT STDMETHODCALLTYPE QueryInterface(
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject);
	ULONG STDMETHODCALLTYPE AddRef(void);
	ULONG STDMETHODCALLTYPE Release(void);
};