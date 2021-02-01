#include "Renderer.h"

HRESULT GetVideoSourceFromFile(LPCWSTR path, IMFMediaSource** ppVideoSource, IMFSourceReader** ppVideoReader);
HRESULT CopyAttribute(IMFAttributes* pSrc, IMFAttributes* pDest, const GUID& key);
#define MEDIA_FILE_PATH L"C:\\Users\\aaron\\OneDrive - University of Derby\\Year 3\\Independant Technology Project\\Main Project\\Test video\\fps_counter_yuv420p.yuv"

//Renderer Constructor
Renderer::Renderer(unsigned __int32 m_width, unsigned __int32 m_height, unsigned __int32 m_bitDepth, HWND m_hwnd)
{
	//TODO move these to the definition
	IMFMediaSource* pVideoSource = NULL;
	IMFSourceReader* pVideoReader = NULL;
	IMFMediaType* videoSourceOutputType = NULL, * pvideoSourceModType = NULL;
	IMFMediaType* pVideoSourceOutType = NULL, * pImfEvrSinkType = NULL;
	IMFMediaType* pHintMediaType = NULL;
	IMFMediaSink* pVideoSink = NULL;
	IMFStreamSink* pStreamSink = NULL;
	IMFMediaTypeHandler* pSinkMediaTypeHandler = NULL, * pSourceMediaTypeHandler = NULL;
	IMFPresentationDescriptor* pSourcePresentationDescriptor = NULL;
	IMFStreamDescriptor* pSourceStreamDescriptor = NULL;
	IMFVideoRenderer* pVideoRenderer = NULL;
	IMFVideoDisplayControl* pVideoDisplayControl = NULL;
	IMFGetService* pService = NULL;
	IMFActivate* pActive = NULL;
	IMFPresentationClock* pClock = NULL;
	IMFPresentationTimeSource* pTimeSource = NULL;
	IDirect3DDeviceManager9* pD3DManager = NULL;
	IMFVideoSampleAllocator* pVideoSampleAllocator = NULL;
	IMFSample* pD3DVideoSample = NULL;
	IMF2DBuffer* p2DBuffer = NULL;
	IMFMediaBuffer* pDstBuffer = NULL;
	RECT rc = { 0, 0, m_width, m_height };
	BOOL fSelected = false;

	IMFMediaEventGenerator* pEventGenerator = NULL;
	IMFMediaEventGenerator* pstreamSinkEventGenerator = NULL;
	MediaEventHandler mediaEvtHandler;
	MediaEventHandler streamSinkMediaEvtHandler;

    DWORD sinkMediaTypeCount = 0;
    DWORD srcMediaTypeCount = 0;

    IMFSample* videoSample = NULL;
    IMFMediaBuffer* pSrcBuffer = NULL;
    BYTE* pbBuffer = NULL;
    DWORD streamIndex, flags;
    LONGLONG llTimeStamp;
    UINT32 uiAttribute = 0;
    DWORD dwBuffer = 0;

    

    CHECK_HR(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE),
        L"COM initialisation failed.");

    CHECK_HR(MFStartup(MF_VERSION),
        L"Media Foundation initialisation failed.");


    // ----- Set up Video sink (Enhanced Video Renderer). -----

    CHECK_HR(MFCreateVideoRendererActivate(m_hwnd, &pActive),
        L"Failed to created video rendered activation context.");

    CHECK_HR(pActive->ActivateObject(IID_IMFMediaSink, (void**)&pVideoSink),
        L"Failed to activate IMFMediaSink interface on video sink.");

    // Initialize the renderer before doing anything else including querying for other interfaces,
    // see https://msdn.microsoft.com/en-us/library/windows/desktop/ms704667(v=vs.85).aspx.
    CHECK_HR(pVideoSink->QueryInterface(__uuidof(IMFVideoRenderer), (void**)&pVideoRenderer),
        L"Failed to get video Renderer interface from EVR media sink.");

    CHECK_HR(pVideoRenderer->InitializeRenderer(NULL, NULL),
        L"Failed to initialise the video renderer.");

    CHECK_HR(pVideoSink->QueryInterface(__uuidof(IMFGetService), (void**)&pService),
        L"Failed to get service interface from EVR media sink.");

    CHECK_HR(pService->GetService(MR_VIDEO_RENDER_SERVICE, __uuidof(IMFVideoDisplayControl), (void**)&pVideoDisplayControl),
        L"Failed to get video display control interface from service interface.");

    CHECK_HR(pVideoDisplayControl->SetVideoWindow(m_hwnd),
        L"Failed to SetVideoWindow.");

    CHECK_HR(pVideoDisplayControl->SetVideoPosition(NULL, &rc),
        L"Failed to SetVideoPosition.");

    CHECK_HR(pVideoSink->GetStreamSinkByIndex(0, &pStreamSink),
        L"Failed to get video renderer stream by index.");

    CHECK_HR(pStreamSink->GetMediaTypeHandler(&pSinkMediaTypeHandler),
        L"Failed to get media type handler for stream sink.");

    
    CHECK_HR(pSinkMediaTypeHandler->GetMediaTypeCount(&sinkMediaTypeCount),
        L"Failed to get sink media type count.");

    // ----- Set up Video source. -----

    CHECK_HR(GetVideoSourceFromFile(MEDIA_FILE_PATH, &pVideoSource, &pVideoReader),
        L"Failed to get file video source.");

    CHECK_HR(pVideoReader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, false),
        L"Failed to de-select all streams on video reader.");

    CHECK_HR(pVideoReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, &videoSourceOutputType),
       L"Error retrieving current media type from first video stream.");

    CHECK_HR(pVideoReader->SetStreamSelection((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, TRUE),
        L"Failed to set the first video stream on the source reader.");

    CHECK_HR(pVideoSource->CreatePresentationDescriptor(&pSourcePresentationDescriptor),
        L"Failed to create the presentation descriptor from the media source.");

    CHECK_HR(pSourcePresentationDescriptor->GetStreamDescriptorByIndex(0, &fSelected, &pSourceStreamDescriptor),
        L"Failed to get source stream descriptor from presentation descriptor.");

    CHECK_HR(pSourceStreamDescriptor->GetMediaTypeHandler(&pSourceMediaTypeHandler),
        L"Failed to get source media type handler.");

    
    CHECK_HR(pSourceMediaTypeHandler->GetMediaTypeCount(&srcMediaTypeCount),
        L"Failed to get source media type count.");

    // ----- Create a compatible media type and set on the source and sink. -----

   // Set the video output type on the file source.
    CHECK_HR(MFCreateMediaType(&pVideoSourceOutType), L"Failed to create video output media type.");
    CHECK_HR(pVideoSourceOutType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video), L"Failed to set video output media major type.");
    CHECK_HR(pVideoSourceOutType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32), L"Failed to set video sub-type attribute on media type.");
    CHECK_HR(pVideoSourceOutType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive), L"Failed to set interlace mode attribute on media type.");
    CHECK_HR(pVideoSourceOutType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE), L"Failed to set independent samples attribute on media type.");
    CHECK_HR(MFSetAttributeRatio(pVideoSourceOutType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1), L"Failed to set pixel aspect ratio attribute on media type.");
    CHECK_HR(CopyAttribute(videoSourceOutputType, pVideoSourceOutType, MF_MT_FRAME_SIZE), L"Failed to copy video frame size attribute to media type.");
    CHECK_HR(CopyAttribute(videoSourceOutputType, pVideoSourceOutType, MF_MT_FRAME_RATE), L"Failed to copy video frame rate attribute to media type.");

    // Set the video input type on the EVR sink.
    CHECK_HR(MFCreateMediaType(&pImfEvrSinkType), L"Failed to create video output media type.");
    CHECK_HR(pImfEvrSinkType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video), L"Failed to set video output media major type.");
    CHECK_HR(pImfEvrSinkType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32), L"Failed to set video sub-type attribute on media type.");
    CHECK_HR(pImfEvrSinkType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive), L"Failed to set interlace mode attribute on media type.");
    CHECK_HR(pImfEvrSinkType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE), L"Failed to set independent samples attribute on media type.");
    CHECK_HR(MFSetAttributeRatio(pImfEvrSinkType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1), L"Failed to set pixel aspect ratio attribute on media type.");
    CHECK_HR(CopyAttribute(videoSourceOutputType, pImfEvrSinkType, MF_MT_FRAME_SIZE), L"Failed to copy video frame size attribute to media type.");
    CHECK_HR(CopyAttribute(videoSourceOutputType, pImfEvrSinkType, MF_MT_FRAME_RATE), L"Failed to copy video frame rate attribute to media type.");
    CHECK_HR(pSinkMediaTypeHandler->SetCurrentMediaType(pImfEvrSinkType),
        L"Failed to set input media type on EVR sink.");

    CHECK_HR(pVideoReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, pVideoSourceOutType),
        L"Failed to set output media type on source reader.");

    // ----- Set up event handler for sink events otherwise memory leaks. -----

    CHECK_HR(pVideoSink->QueryInterface(IID_IMFMediaEventGenerator, (void**)&pEventGenerator),
       L"Video sink doesn't support IMFMediaEventGenerator interface.");

    CHECK_HR(pEventGenerator->BeginGetEvent((IMFAsyncCallback*)&mediaEvtHandler, pEventGenerator),
       L"BeginGetEvent on media generator failed.");

    CHECK_HR(pStreamSink->QueryInterface(IID_IMFMediaEventGenerator, (void**)&pstreamSinkEventGenerator),
       L"Stream sink doesn't support IMFMediaEventGenerator interface.");

    CHECK_HR(pstreamSinkEventGenerator->BeginGetEvent((IMFAsyncCallback*)&streamSinkMediaEvtHandler, pstreamSinkEventGenerator),
       L"BeginGetEvent on stream sink media generator failed.");

    // ----- Source and sink now configured. Set up remaining infrastructure and then start sampling. -----

    // Get Direct3D surface organised.
    // https://msdn.microsoft.com/fr-fr/library/windows/desktop/aa473823(v=vs.85).aspx
    CHECK_HR(MFGetService(pStreamSink, MR_VIDEO_ACCELERATION_SERVICE, IID_PPV_ARGS(&pVideoSampleAllocator)), L"Failed to get IMFVideoSampleAllocator.");
    CHECK_HR(MFGetService(pVideoSink, MR_VIDEO_ACCELERATION_SERVICE, IID_PPV_ARGS(&pD3DManager)), L"Failed to get Direct3D manager from EVR media sink.");
    CHECK_HR(pVideoSampleAllocator->SetDirectXManager(pD3DManager), L"Failed to set D3DManager on video sample allocator.");
    CHECK_HR(pVideoSampleAllocator->InitializeSampleAllocator(1, pImfEvrSinkType), L"Failed to initialise video sample allocator.");
    CHECK_HR(pVideoSampleAllocator->AllocateSample(&pD3DVideoSample), L"Failed to allocate video sample.");
    CHECK_HR(pD3DVideoSample->GetBufferByIndex(0, &pDstBuffer), L"Failed to get destination buffer.");
    CHECK_HR(pDstBuffer->QueryInterface(IID_PPV_ARGS(&p2DBuffer)), L"Failed to get pointer to 2D buffer.");

    // Get clocks organised.
    CHECK_HR(MFCreatePresentationClock(&pClock), L"Failed to create presentation clock.");
    CHECK_HR(MFCreateSystemTimeSource(&pTimeSource), L"Failed to create system time source.");
    CHECK_HR(pClock->SetTimeSource(pTimeSource), L"Failed to set time source.");
    CHECK_HR(pVideoSink->SetPresentationClock(pClock), L"Failed to set presentation clock on video sink.");
    CHECK_HR(pClock->Start(0),L"Error starting presentation clock.");

    // Start the sample read-write loop.
   

    while (true)
    {
        CHECK_HR(pVideoReader->ReadSample(
            MF_SOURCE_READER_FIRST_VIDEO_STREAM,
            0,                              // Flags.
            &streamIndex,                   // Receives the actual stream index. 
            &flags,                         // Receives status flags.
            &llTimeStamp,                   // Receives the time stamp.
            &videoSample                    // Receives the sample or NULL.
        ),L"Error reading video sample.");

        if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
        {
            printf("End of stream.\n");

            PROPVARIANT var = { 0 };
            var.vt = VT_I8;

            CHECK_HR(pVideoReader->SetCurrentPosition(GUID_NULL, var),
                L"Failed to set source reader position.");
        }
        if (flags & MF_SOURCE_READERF_STREAMTICK)
        {
            printf("Stream tick.\n");
        }

        if (!videoSample)
        {
            printf("Null video sample.\n");
        }
        else
        {
            UINT sampleCount = 0;
            LONGLONG sampleDuration = 0;
            CHECK_HR(videoSample->GetCount(&sampleCount), L"Failed to get video sample count.");
            CHECK_HR(videoSample->GetSampleDuration(&sampleDuration), L"Failed to get video sample duration.");

            //printf("Attempting to write sample to stream sink, sample count %d, sample duration %llu, sample time %llu.\n", sampleCount, sampleDuration, llTimeStamp);

            CHECK_HR(pD3DVideoSample->SetSampleTime(llTimeStamp), L"Failed to set D3D video sample time.");
            CHECK_HR(pD3DVideoSample->SetSampleDuration(sampleDuration), L"Failed to set D3D video sample duration.");
            CHECK_HR(videoSample->ConvertToContiguousBuffer(&pSrcBuffer), L"Failed to get buffer from video sample.");
            CHECK_HR(pSrcBuffer->Lock(&pbBuffer, NULL, &dwBuffer), L"Failed to lock sample buffer.");
            CHECK_HR(p2DBuffer->ContiguousCopyFrom(pbBuffer, dwBuffer), L"Failed to unlock sample buffer.");
            CHECK_HR(pSrcBuffer->Unlock(), L"Failed to unlock source buffer.\n");

            CHECK_HR(videoSample->GetUINT32(MFSampleExtension_FrameCorruption, &uiAttribute), L"Failed to get frame corruption attribute.");
            CHECK_HR(pD3DVideoSample->SetUINT32(MFSampleExtension_FrameCorruption, uiAttribute), L"Failed to set frame corruption attribute.");
            CHECK_HR(videoSample->GetUINT32(MFSampleExtension_Discontinuity, &uiAttribute), L"Failed to get discontinuity attribute.");
            CHECK_HR(pD3DVideoSample->SetUINT32(MFSampleExtension_Discontinuity, uiAttribute), L"Failed to set discontinuity attribute.");
            CHECK_HR(videoSample->GetUINT32(MFSampleExtension_CleanPoint, &uiAttribute), L"Failed to get clean point attribute.");
            CHECK_HR(pD3DVideoSample->SetUINT32(MFSampleExtension_CleanPoint, uiAttribute), L"Failed to set clean point attribute.");

            CHECK_HR(pStreamSink->ProcessSample(pD3DVideoSample),L"Stream sink process sample failed.");

            Sleep(sampleDuration / 10000); // Duration is given in 100's of nano seconds.
        }

        SAFE_RELEASE(pSrcBuffer);
        SAFE_RELEASE(videoSample); 
    }

	done:
    SAFE_RELEASE(p2DBuffer);
    SAFE_RELEASE(pDstBuffer);
    SAFE_RELEASE(pVideoReader);
    SAFE_RELEASE(videoSourceOutputType);
    SAFE_RELEASE(pvideoSourceModType);
    SAFE_RELEASE(pImfEvrSinkType);
    SAFE_RELEASE(pHintMediaType);
    SAFE_RELEASE(pVideoSink);
    SAFE_RELEASE(pStreamSink);
    SAFE_RELEASE(pSinkMediaTypeHandler);
    SAFE_RELEASE(pSourceMediaTypeHandler);
    SAFE_RELEASE(pSourcePresentationDescriptor);
    SAFE_RELEASE(pSourceStreamDescriptor);
    SAFE_RELEASE(pVideoRenderer);
    SAFE_RELEASE(pVideoDisplayControl);
    SAFE_RELEASE(pService);
    SAFE_RELEASE(pActive);
    SAFE_RELEASE(pClock);
    SAFE_RELEASE(pTimeSource);
    SAFE_RELEASE(pD3DManager);
    SAFE_RELEASE(pVideoSampleAllocator);
    SAFE_RELEASE(pD3DVideoSample);
    SAFE_RELEASE(pEventGenerator);
    SAFE_RELEASE(pstreamSinkEventGenerator);

	ShutdownRenderer();
}

HRESULT Renderer::ShutdownRenderer()
{
	//TODO shutdown the renderer and release everything
    
    CoUninitialize();
	return 0;
}

HRESULT Renderer::SetVideoSource()
{
	//TODO set the video source
	return 0;
}

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

/**
* Gets a video source reader from a media file.
* @param[in] path: the media file path to get the source reader for.
* @param[out] ppVideoSource: will be set with the source for the reader if successful.
* @param[out] ppVideoReader: will be set with the reader if successful.
* @@Returns S_OK if successful or an error code if not.
*/
HRESULT GetVideoSourceFromFile(LPCWSTR path, IMFMediaSource** ppVideoSource, IMFSourceReader** ppVideoReader)
{
    IMFSourceResolver* pSourceResolver = NULL;
    IUnknown* uSource = NULL;

    IMFAttributes* pVideoReaderAttributes = NULL;
    MF_OBJECT_TYPE ObjectType = MF_OBJECT_INVALID;

    HRESULT hr = S_OK;

    hr = MFCreateSourceResolver(&pSourceResolver);
    CHECK_HR(hr, L"MFCreateSourceResolver failed.");

    hr = pSourceResolver->CreateObjectFromURL(
        path,                       // URL of the source.
        MF_RESOLUTION_MEDIASOURCE,  // Create a source object.
        NULL,                       // Optional property store.
        &ObjectType,                // Receives the created object type. 
        &uSource                    // Receives a pointer to the media source. 
    );
    CHECK_HR(hr, L"Failed to create media source resolver for file.");

    hr = uSource->QueryInterface(IID_PPV_ARGS(ppVideoSource));
    CHECK_HR(hr, L"Failed to create media file source.");

    hr = MFCreateAttributes(&pVideoReaderAttributes, 1);
    CHECK_HR(hr, L"Failed to create attributes object for video reader.");

    hr = pVideoReaderAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, 1);
    CHECK_HR(hr, L"Failed to set enable video processing attribute type for reader config.");

    hr = MFCreateSourceReaderFromMediaSource(*ppVideoSource, pVideoReaderAttributes, ppVideoReader);
    CHECK_HR(hr, L"Error creating media source reader.");

done:

    SAFE_RELEASE(pSourceResolver);
    SAFE_RELEASE(uSource);
    SAFE_RELEASE(pVideoReaderAttributes);

    return hr;
}

HRESULT CopyAttribute(IMFAttributes* pSrc, IMFAttributes* pDest, const GUID& key)
{
    PROPVARIANT var;
    PropVariantInit(&var);

    HRESULT hr = S_OK;

    hr = pSrc->GetItem(key, &var);
    if (SUCCEEDED(hr))
    {
        hr = pDest->SetItem(key, var);
    }

    PropVariantClear(&var);
    return hr;
}