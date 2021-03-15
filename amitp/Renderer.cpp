#include "Renderer.h"


void checkRTPSessError(int rtperr)
{
	if (rtperr < 0)
	{
		std::cout << "ERROR: " << jrtplib::RTPGetErrorString(rtperr) << std::endl;
		exit(-1);
	}
}

//Renderer Constructor
Renderer::Renderer(unsigned __int32 width, unsigned __int32 height, unsigned __int32 bitDepth, HWND hwnd) : RTPSession(), BufferQueues(2321280)
{
	//move all to initialisation list so its not written twice https://stackoverflow.com/questions/6822422/c-where-to-initialize-variables-in-constructor
    sourceFormat.width = width;
    sourceFormat.height = height;
    sourceFormat.bitDepth = bitDepth;
	
    m_hwnd = hwnd;

	IMFMediaType* pVideoOutType = NULL;
	IMFMediaSink* pVideoSink = NULL;
	IMFStreamSink* pStreamSink = NULL;
	IMFMediaTypeHandler* pSinkMediaTypeHandler = NULL;
	IMFVideoRenderer* pVideoRenderer = NULL;
	IMFVideoDisplayControl* pVideoDisplayControl = NULL;
	IMFGetService* pService = NULL;
	IMFActivate* pActive = NULL;
	IMFPresentationClock* pClock = NULL;
	IMFPresentationTimeSource* pTimeSource = NULL;
	IDirect3DDeviceManager9* pD3DManager = NULL;
	IMFVideoSampleAllocator* pVideoSampleAllocator = NULL;
	IMFSample* pD3DVideoSample = NULL;
	IMFMediaBuffer* pDstBuffer = NULL;
	IMF2DBuffer* p2DBuffer = NULL;
	RECT rc = { 0, 0, width, height };
	BOOL fSelected = false;
	BYTE* bitmapBuffer = new BYTE[4 * width * width];;

	IMFMediaEventGenerator* pEventGenerator = NULL;
	IMFMediaEventGenerator* pstreamSinkEventGenerator = NULL;
	MediaEventHandler mediaEvtHandler;
	MediaEventHandler streamSinkMediaEvtHandler;
}

void Renderer::startRTPReceiver(uint16_t port)
{
	firstMarker = true;
	lastPacketHadMarker = false;
	firstFrame = true;
	WSAStartup(MAKEWORD(2, 2), &dat);

	sessparams.SetOwnTimestampUnit(1.0 / 8000.);
	transparams.SetPortbase(portbase);
	transparams.SetRTPReceiveBuffer(174483046*2);
	
	status = Create(sessparams, &transparams);
	checkRTPSessError(status);
}

void Renderer::stopRTPReceiver()
{
	BYEDestroy(jrtplib::RTPTime(10, 0), 0, 0);
	WSACleanup();
}

void Renderer::startRenderer()
{
	DWORD sinkMediaTypeCount = 0;
	unsigned char renderBuffer[2321280];
	size_t renderBufferLength = 2321280;
	LONGLONG llTimeStamp = 0;
	UINT bitmapCount = 0;
	LONGLONG sampleDuration = 333333;
	
	CHECK_HR(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE),
		"COM initialisation failed.");

	CHECK_HR(MFStartup(MF_VERSION),
		"Media Foundation initialisation failed.");


	// ----- Set up Video sink (Enhanced Video Renderer). -----

	CHECK_HR(MFCreateVideoRendererActivate(m_hwnd, &pActive),
		"Failed to created video rendered activation context.");

	CHECK_HR(pActive->ActivateObject(IID_IMFMediaSink, (void**)&pVideoSink),
		"Failed to activate IMFMediaSink interface on video sink.");

	// Initialize the renderer before doing anything else including querying for other interfaces,
	// see https://msdn.microsoft.com/en-us/library/windows/desktop/ms704667(v=vs.85).aspx.
	CHECK_HR(pVideoSink->QueryInterface(__uuidof(IMFVideoRenderer), (void**)&pVideoRenderer),
		"Failed to get video Renderer interface from EVR media sink.");

	CHECK_HR(pVideoRenderer->InitializeRenderer(NULL, NULL),
		"Failed to initialise the video renderer.");

	CHECK_HR(pVideoSink->QueryInterface(__uuidof(IMFGetService), (void**)&pService),
		"Failed to get service interface from EVR media sink.");

	CHECK_HR(pService->GetService(MR_VIDEO_RENDER_SERVICE, __uuidof(IMFVideoDisplayControl), (void**)&pVideoDisplayControl),
		"Failed to get video display control interface from service interface.");

	CHECK_HR(pVideoDisplayControl->SetVideoWindow(m_hwnd),
		"Failed to SetVideoWindow.");

	CHECK_HR(pVideoDisplayControl->SetVideoPosition(NULL, &rc),
		"Failed to SetVideoPosition.");

	CHECK_HR(pVideoSink->GetStreamSinkByIndex(0, &pStreamSink),
		"Failed to get video renderer stream by index.");

	CHECK_HR(pStreamSink->GetMediaTypeHandler(&pSinkMediaTypeHandler),
		"Failed to get media type handler for stream sink.");

	
	CHECK_HR(pSinkMediaTypeHandler->GetMediaTypeCount(&sinkMediaTypeCount),
		"Failed to get sink media type count.");

	std::cout << "Sink media type count: " << sinkMediaTypeCount << "." << std::endl;

	// ----- Create the EVR compatible media type and set on the stream sink. -----

	// Set the video input type on the EVR sink.
	CHECK_HR(MFCreateMediaType(&pVideoOutType), "Failed to create video output media type.");
	CHECK_HR(pVideoOutType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video), "Failed to set video output media major type.");
	CHECK_HR(pVideoOutType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_Y210), "Failed to set video sub-type attribute on media type.");
	CHECK_HR(pVideoOutType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive), "Failed to set interlace mode attribute on media type.");
	CHECK_HR(pVideoOutType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE), "Failed to set independent samples attribute on media type.");
	CHECK_HR(MFSetAttributeRatio(pVideoOutType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1), "Failed to set pixel aspect ratio attribute on media type.");
	CHECK_HR(MFSetAttributeSize(pVideoOutType, MF_MT_FRAME_SIZE, sourceFormat.width, sourceFormat.height), "Failed to set the frame size attribute on media type.");

	//std::cout << "EVR input media type defined as:" << std::endl;
	//std::cout << GetMediaTypeDescription(pVideoOutType) << std::endl << std::endl;

	CHECK_HR(pSinkMediaTypeHandler->SetCurrentMediaType(pVideoOutType),
		"Failed to set input media type on EVR sink.");

	// ----- Set up event handler for sink events otherwise memory leaks. -----

	CHECK_HR(pVideoSink->QueryInterface(IID_IMFMediaEventGenerator, (void**)&pEventGenerator),
		"Video sink doesn't support IMFMediaEventGenerator interface.");

	CHECK_HR(pEventGenerator->BeginGetEvent((IMFAsyncCallback*)&mediaEvtHandler, pEventGenerator),
		"BeginGetEvent on video sink media generator failed.");

	CHECK_HR(pStreamSink->QueryInterface(IID_IMFMediaEventGenerator, (void**)&pstreamSinkEventGenerator),
		"Stream sink doesn't support IMFMediaEventGenerator interface.");

	CHECK_HR(pstreamSinkEventGenerator->BeginGetEvent((IMFAsyncCallback*)&streamSinkMediaEvtHandler, pstreamSinkEventGenerator),
		"BeginGetEvent on stream sink media generator failed.");

	// Get Direct3D surface organised.
	// https://msdn.microsoft.com/fr-fr/library/windows/desktop/aa473823(v=vs.85).aspx
	CHECK_HR(MFGetService(pStreamSink, MR_VIDEO_ACCELERATION_SERVICE, IID_PPV_ARGS(&pVideoSampleAllocator)), "Failed to get IMFVideoSampleAllocator.");
	CHECK_HR(MFGetService(pVideoSink, MR_VIDEO_ACCELERATION_SERVICE, IID_PPV_ARGS(&pD3DManager)), "Failed to get Direct3D manager from EVR media sink.");
	CHECK_HR(pVideoSampleAllocator->SetDirectXManager(pD3DManager), "Failed to set D3DManager on video sample allocator.");
	CHECK_HR(pVideoSampleAllocator->InitializeSampleAllocator(1, pVideoOutType), "Failed to initialise video sample allocator.");
	CHECK_HR(pVideoSampleAllocator->AllocateSample(&pD3DVideoSample), "Failed to allocate video sample.");
	CHECK_HR(pD3DVideoSample->GetBufferByIndex(0, &pDstBuffer), "Failed to get destination buffer.");
	CHECK_HR(pDstBuffer->QueryInterface(IID_PPV_ARGS(&p2DBuffer)), "Failed to get pointer to 2D buffer.");

	// Get clocks organised.
	CHECK_HR(MFCreatePresentationClock(&pClock), "Failed to create presentation clock.");
	CHECK_HR(MFCreateSystemTimeSource(&pTimeSource), "Failed to create system time source.");
	CHECK_HR(pClock->SetTimeSource(pTimeSource), "Failed to set time source.");
	CHECK_HR(pVideoSink->SetPresentationClock(pClock), "Failed to set presentation clock on video sink.");
	CHECK_HR(pClock->Start(0), "Error starting presentation clock.");

	// Start writing bitmaps.
	{
		std::lock_guard<std::mutex> lock(m);
		if (!frameQueue.empty())
		{
			//if (frameQueue.begin()->frameComplete)
			{
				//iterate through the frameBuffer and copy to the render buffer
				size_t transfered = 0;
				for (int i = 0; i < frameQueue.begin()->frameData.size(); i++)
				{
					memcpy(&renderBuffer[transfered], &frameQueue.begin()->frameData[i].payload, frameQueue.begin()->frameData[i].payloadLength);
					transfered += frameQueue.begin()->frameData[i].payloadLength;
				}
				CHECK_HR(pD3DVideoSample->SetSampleTime(llTimeStamp), "Failed to set D3D video sample time.");
				CHECK_HR(pD3DVideoSample->SetSampleDuration(sampleDuration), "Failed to set D3D video sample duration.");
				CHECK_HR(p2DBuffer->ContiguousCopyFrom(renderBuffer, renderBufferLength), "Failed to copy frame to D2D buffer.");
				CHECK_HR(pStreamSink->ProcessSample(pD3DVideoSample), "Stream sink process sample failed.");
			}
		}
		
	}

	//while (true)
	//{
	//	{
	//		std::lock_guard<std::mutex> lock(m);
	//		if (frameQueue.begin()->frameComplete)
	//		{
	//			CHECK_HR(pD3DVideoSample->SetSampleTime(llTimeStamp), "Failed to set D3D video sample time.");
	//			CHECK_HR(pD3DVideoSample->SetSampleDuration(sampleDuration), "Failed to set D3D video sample duration.");
	//			CHECK_HR(p2DBuffer->ContiguousCopyFrom(renderBuffer, renderBufferLength), "Failed to copy bitmap to D2D buffer.");
	//			CHECK_HR(pStreamSink->ProcessSample(pD3DVideoSample), "Stream sink process sample failed.");
	//		}
	//	}

	//	//Sleep(SAMPLE_DURATION / 10000);

	//	llTimeStamp += sampleDuration;
	//}

done:

	printf("finished.\n");
	auto c = getchar();

	delete[] bitmapBuffer;
	SAFE_RELEASE(p2DBuffer);
	SAFE_RELEASE(pDstBuffer);
	SAFE_RELEASE(pVideoOutType);
	SAFE_RELEASE(pVideoSink);
	SAFE_RELEASE(pStreamSink);
	SAFE_RELEASE(pSinkMediaTypeHandler);
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

	return;
}


void Renderer::OnPollThreadStep()
{
	BeginDataAccess();

	// check incoming packets
	if (GotoFirstSourceWithData())
	{
		do
		{
			jrtplib::RTPPacket* pack;
			
			while ((pack = GetNextPacket()) != NULL)
			{
				if (firstMarker && !pack->HasMarker()) //only proccess the packet at the start if its useful
				{
					goto notUsefullPacket;
				} else
				{
					ProcessRTPPacket(*pack);
				}
				
			}
			notUsefullPacket: DeletePacket(pack);
			
		} while (GotoNextSourceWithData());
	}

	EndDataAccess();
}

void Renderer::ProcessRTPPacket(const jrtplib::RTPPacket& rtppack)
{
	if (firstMarker && rtppack.HasMarker())
	{
		firstMarker = false;
		lastPacketHadMarker = true;
		goto done;
	}
	if (lastPacketHadMarker)
	{
		lastPacketHadMarker = false;
		addFrame(rtppack.GetTimestamp());
		//addPayload(rtppack.GetPayloadData(), rtppack.GetPayloadLength(), rtppack.GetTimestamp(), rtppack.GetExtendedSequenceNumber());
	} else if (rtppack.HasMarker())
	{
		lastPacketHadMarker = true;
		//addPayload(rtppack.GetPayloadData(), rtppack.GetPayloadLength(), rtppack.GetTimestamp(), rtppack.GetExtendedSequenceNumber());
	}
	addPayload(rtppack.GetPayloadData(), rtppack.GetPayloadLength(), rtppack.GetTimestamp(), rtppack.GetExtendedSequenceNumber());
done: return;
}

    
  




