//TODO create structure to describe the source format
//TODO create class that handles the video rendering

#include "Renderer.h"

//Renderer Constructor TODO add hwnd han
Renderer::Renderer(unsigned __int32 m_width, unsigned __int32 m_height, unsigned __int32 m_bit_depth, HWND m_hwnd)
{
	//create the format structure and get a pointer to it
	struct sourceFormatDescriptor m_sourceFormat = {m_width, m_height, m_bit_depth}, * pSourceFormat = &m_sourceFormat;

	IMFActivate* m_pRendererActivate = NULL;
	IMFMediaSink* m_pVideoSink = NULL;
	IMFVideoRenderer* m_pVideoRenderer = NULL;
	IMFGetService* m_pService = NULL;
	IMFVideoDisplayControl* m_pVideoDisplayControl = NULL;
	IMFStreamSink* m_pStreamSink = NULL;
	IMFMediaTypeHandler* m_pSinkMediaTypeHandler = NULL, * m_pSourceMediaTypeHandler = NULL;


	RECT m_rc = { 0,0, m_width, m_width };


	HRESULT hr;
	
	//enable asynchronous activities in MF
	if (!SUCCEEDED(hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
	{
		OutputDebugStringW(L"CoInitialiseEx failed\n");
		return; //TODO replace this with some proper error catching
		
	}

	//initialise the MF instance
	if (!SUCCEEDED(hr = MFStartup(MF_VERSION)))
	{
		OutputDebugStringW(L"MF start up failed\n");
		return; //TODO replace this with some proper error catching
		
	}

	//set up the video sink (EVR)
	if (!SUCCEEDED(hr = MFCreateVideoRendererActivate(m_hwnd, &m_pRendererActivate)))
	{
		OutputDebugStringW(L"Create Video Renderer failed\n");
		return; //TODO replace this with some proper error catching
		
	}

	if (!SUCCEEDED(hr = m_pRendererActivate->ActivateObject(IID_IMFMediaSink, (void**)&m_pVideoSink)))
	{
		OutputDebugStringW(L"Failed to activate IMFMediaSink on video sink\n");
		return; //TODO replace this with some proper error catching
		
	}

	if (!SUCCEEDED(hr = m_pVideoSink->QueryInterface(__uuidof(IMFVideoRenderer), (void**)&m_pVideoRenderer)))
	{
		OutputDebugStringW(L"Failed to get the EVR interface from the media sink\n");
		return; //TODO replace this with some proper error catching
		
	}

	if (!SUCCEEDED(hr = m_pVideoRenderer->InitializeRenderer(NULL, NULL)))
	{
		OutputDebugStringW(L"Failed to intialise the video renderer\n");
		return; //TODO replace this with some proper error catching
		
	}

	if (!SUCCEEDED(hr = m_pVideoSink->QueryInterface(__uuidof(IMFGetService), (void**)&m_pService)))
	{
		OutputDebugStringW(L"Failed to get service interface from the EVR media sink\n");
		return; //TODO replace this with some proper error catching
		
	}

	if (!SUCCEEDED(hr = m_pService->GetService(MR_VIDEO_RENDER_SERVICE, __uuidof(IMFVideoDisplayControl), (void**)&m_pVideoDisplayControl)))
	{
		OutputDebugStringW(L"Failed to get video display control interface from service interface\n");
		return; //TODO replace this with some proper error catching
		
	}

	if (!SUCCEEDED(hr = m_pVideoDisplayControl->SetVideoWindow(m_hwnd)))
	{
		OutputDebugStringW(L"Failed to SetVideoWindow\n");
		return; //TODO replace this with some proper error catching
		
	}

	if (!SUCCEEDED(hr = m_pVideoDisplayControl->SetVideoPosition(NULL, &m_rc)))
	{
		OutputDebugStringW(L"Failed to SetVideoPosition\n");
		return; //TODO replace this with some proper error catching
		
	}


	if (!SUCCEEDED(hr = m_pVideoSink->GetStreamSinkByIndex(0, &m_pStreamSink)))
	{
		OutputDebugStringW(L"Failed to get video renderer stream by index\n");
		return; //TODO replace this with some proper error catching
		
	}

	if (!SUCCEEDED(hr = m_pStreamSink->GetMediaTypeHandler(&m_pSinkMediaTypeHandler)))
	{
		OutputDebugStringW(L"Failed to get source media type handler.\n");
		return; //TODO replace this with some proper error catching
		
	}

	DWORD m_sinkMediaTypeCount = 0;
	if (!SUCCEEDED(hr = m_pSinkMediaTypeHandler->GetMediaTypeCount(&m_sinkMediaTypeCount)))
	{
		OutputDebugStringW(L"Failed to get source media type count\n");
		return; //TODO replace this with some proper error catching
		
	}

	OutputDebugStringW(L"Load Complete\n");

}

