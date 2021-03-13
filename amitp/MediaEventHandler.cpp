#include "MediaEventHandler.h"
#include "commonTools.h"

HRESULT MediaEventHandler::Invoke(IMFAsyncResult* pAsyncResult)
{
    HRESULT hr = S_OK;
    IMFMediaEvent* pEvent = NULL;
    MediaEventType meType = MEUnknown;
    BOOL fGetAnotherEvent = TRUE;
    HRESULT hrStatus = S_OK;
    IMFMediaEventGenerator* pEventGenerator = NULL;

    hr = pAsyncResult->GetState((IUnknown**)&pEventGenerator);
    if (!SUCCEEDED(hr))
    {
        OutputDebugStringW(L"Failed to get media event generator from async state.\n");
    }

    // Get the event from the event queue.
    // Assume that m_pEventGenerator is a valid pointer to the
    // event generator's IMFMediaEventGenerator interface.
    hr = pEventGenerator->EndGetEvent(pAsyncResult, &pEvent);

    // Get the event type.
    if (SUCCEEDED(hr))
    {
        hr = pEvent->GetType(&meType);
        //printf("Media event type %d.\n", meType);
    }

    // Get the event status. If the operation that triggered the event 
    // did not succeed, the status is a failure code.
    if (SUCCEEDED(hr))
    {
        hr = pEvent->GetStatus(&hrStatus);
    }

    if (SUCCEEDED(hr))
    {
        // TODO: Handle the event.
    }

    // If not finished, request another event.
    // Pass in a pointer to this instance of the application's
    // CEventHandler class, which implements the callback.
    if (fGetAnotherEvent)
    {
        hr = pEventGenerator->BeginGetEvent(this, pEventGenerator);
    }

    SAFE_RELEASE(pEvent);
    return hr;
}

HRESULT  MediaEventHandler::GetParameters(
    DWORD* pdwFlags,
    DWORD* pdwQueue
)
{
    pdwFlags = 0;
    pdwQueue = 0;
    return S_OK;
}

HRESULT  MediaEventHandler::QueryInterface(
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject)
{
    return S_OK;
}

ULONG  MediaEventHandler::AddRef(void)
{
    return 0;
}

ULONG  MediaEventHandler::Release(void)
{
    return 0;
}