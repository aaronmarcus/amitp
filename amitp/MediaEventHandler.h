#pragma once

#ifndef MEDIA_EVENT_HANDLER_H
#define MEDIA_EVENT_HANDLER_H

#include <mfobjects.h>

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

#endif

