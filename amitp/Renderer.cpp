#include "Renderer.h"

#include <bitset>

//JRTPLIB check for RTP errors, and display them to console
void checkRTPSessError(int rtperr)
{
	if (rtperr < 0)
	{
		std::cout << "ERROR: " << jrtplib::RTPGetErrorString(rtperr) << std::endl;
		exit(-1);
	}
}

//function to get the Image stride for the provided types
//returns to stride to pointer of plStride when given the width, height and type
HRESULT GetDefaultStride(IMFMediaType* pType, LONG* plStride, UINT32 width, UINT32 height)
{
	LONG lStride = 0; //temporary stride holder

	// Try to get the default stride from the media type.
	HRESULT hr = pType->GetUINT32(MF_MT_DEFAULT_STRIDE, (UINT32*)&lStride);
	if (FAILED(hr))
	{
		// Attribute not set. Try to calculate the default stride.
		GUID subtype = GUID_NULL;

		// Get the subtype and the image size.
		hr = pType->GetGUID(MF_MT_SUBTYPE, &subtype);
		if (FAILED(hr))
		{
			goto done;
		}

		//get the size of the image from the type, width and height
		hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &width, &height);
		if (FAILED(hr))
		{
			goto done;
		}

		//get the calculated stride
		hr = MFGetStrideForBitmapInfoHeader(subtype.Data1, width, &lStride);
		if (FAILED(hr))
		{
			goto done;
		}

		// Set the attribute for later reference.
		(void)pType->SetUINT32(MF_MT_DEFAULT_STRIDE, UINT32(lStride));
	}

	if (SUCCEEDED(hr))
	{
		*plStride = lStride; //set the stride result onto the variable
	}

done:
	return hr;
}

//function to create a video type for uncompressed video
HRESULT CreateUncompressedVideoType(
	DWORD                fccFormat,  // FOURCC or D3DFORMAT value.     
	UINT32               width,
	UINT32               height,
	MFVideoInterlaceMode interlaceMode,
	const MFRatio& frameRate,
	const MFRatio& par,
	IMFMediaType** ppType
)
{
	if (ppType == NULL) //check to see if a type pointer has been set
	{
		return E_POINTER;
	}

	//set placeholders
	GUID    subtype = MFVideoFormat_Base;
	LONG    lStride = 0;
	UINT32    cbImage = 0;

	//create temporary type holder
	IMFMediaType* pType = NULL;

	// Set the subtype GUID from the FOURCC or D3DFORMAT value.
	subtype.Data1 = fccFormat;

	//create a media type on the temporary type holder
	HRESULT hr = MFCreateMediaType(&pType);
	if (FAILED(hr))
	{
		goto done;
	}

	//set the major type to video
	hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	if (FAILED(hr))
	{
		goto done;
	}

	//set the subtype to the specified type
	hr = pType->SetGUID(MF_MT_SUBTYPE, subtype);
	if (FAILED(hr))
	{
		goto done;
	}

	//set the interlace mode
	hr = pType->SetUINT32(MF_MT_INTERLACE_MODE, interlaceMode);
	if (FAILED(hr))
	{
		goto done;
	}

	//set the frame size
	hr = MFSetAttributeSize(pType, MF_MT_FRAME_SIZE, width, height);
	if (FAILED(hr))
	{
		goto done;
	}

	//get the image stride
	hr = GetDefaultStride(pType, &lStride, width, height);
	if (FAILED(hr))
	{
		goto done;
	}

	//set the image stride
	hr = pType->SetUINT32(MF_MT_DEFAULT_STRIDE, UINT32(lStride));
	if (FAILED(hr))
	{
		goto done;
	}

	// Calculate the image size in bytes.
	hr = MFCalculateImageSize(subtype, width, height, &cbImage);
	if (FAILED(hr))
	{
		goto done;
	}

	//set the size
	hr = pType->SetUINT32(MF_MT_SAMPLE_SIZE, cbImage);
	if (FAILED(hr))
	{
		goto done;
	}

	//set fixed size samples to true
	hr = pType->SetUINT32(MF_MT_FIXED_SIZE_SAMPLES, TRUE);
	if (FAILED(hr))
	{
		goto done;
	}

	//set all samples independent to true
	hr = pType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
	if (FAILED(hr))
	{
		goto done;
	}

	// set the frame rate
	hr = MFSetAttributeRatio(pType, MF_MT_FRAME_RATE, frameRate.Numerator, frameRate.Denominator);
	if (FAILED(hr))
	{
		goto done;
	}

	// set the Pixel aspect ratio
	hr = MFSetAttributeRatio(pType, MF_MT_PIXEL_ASPECT_RATIO, par.Numerator, par.Denominator);
	if (FAILED(hr))
	{
		goto done;
	}

	// Return the pointer to the caller.
	*ppType = pType;
	(*ppType)->AddRef();

done:
	SAFE_RELEASE(&pType); //release the temporary type holder
	return hr;
}

//Renderer Constructor
Renderer::Renderer(unsigned __int32 width, unsigned __int32 height, unsigned __int32 bitDepth, HWND hwnd) : RTPSession(), BufferQueues(2321280)
{
	//initial the needed variables and objects
	//TODO move all to initialisation list so its not written twice https://stackoverflow.com/questions/6822422/c-where-to-initialize-variables-in-constructor
    sourceFormat.width = width;
    sourceFormat.height = height;
    sourceFormat.bitDepth = bitDepth;
	
    m_hwnd = hwnd;

	pVideoOutType = NULL;
	pVideoSink = NULL;
	pStreamSink = NULL;
	pSinkMediaTypeHandler = NULL;
	pVideoRenderer = NULL;
	pVideoDisplayControl = NULL;
	pService = NULL;
	pActive = NULL;
	pClock = NULL;
	pTimeSource = NULL;
	pD3DManager = NULL;
	pVideoSampleAllocator = NULL;
	pD3DVideoSample = NULL;
	pDstBuffer = NULL;
	p2DBuffer = NULL;
	rc = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
	fSelected = false;

	pEventGenerator = NULL;
	pstreamSinkEventGenerator = NULL;
	mediaEvtHandler;
	streamSinkMediaEvtHandler;
}

//function to start receiving RTP packets, given the port
void Renderer::startRTPReceiver(uint16_t port)
{
	//set tracking variables
	firstMarker = true;
	lastPacketHadMarker = false;
	WSAStartup(MAKEWORD(2, 2), &dat); //start winsock2

	//set the RTP session paramaters
	sessparams.SetOwnTimestampUnit(1.0 / 8000.);
	transparams.SetPortbase(portbase);
	transparams.SetRTPReceiveBuffer(5400000); //make the buffer bigger for video 
	SetMaximumPacketSize(1500); //define the max packet size for efficient use of memory
	
	status = Create(sessparams, &transparams); //create the session
	checkRTPSessError(status); //check for errors
}

//function to stop receiving RTP packets
void Renderer::stopRTPReceiver()
{
	//quit RTPLIB
	BYEDestroy(jrtplib::RTPTime(10, 0), 0, 0);
	WSACleanup(); //release the winsock2 objects
}

//function to start rendering
void Renderer::startRenderer()
{
	//set up needed variables
	stopRTPReceiver();
	DWORD sinkMediaTypeCount = 0;
	size_t renderBufferLength = 3686400;
	LONGLONG llTimeStamp = 0;
	UINT bitmapCount = 0;
	LONGLONG sampleDuration = 333333;
	uint32_t stride = 0;
	MFRatio fps = {30,1}, par = {1,1};
	IMFMediaType* supported;

	//start up Media Foundation
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

	// ----- Create the EVR compatible media type and set on the stream sink. -----

	// Set the video input type on the EVR sink.
	CHECK_HR(CreateUncompressedVideoType('2YUY', sourceFormat.width, sourceFormat.height, MFVideoInterlace_Progressive, fps, par, &pVideoOutType), "Failed to create uncompressed video type");

	CHECK_HR(pSinkMediaTypeHandler->IsMediaTypeSupported(pVideoOutType, &supported), "Media Type not supported");

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

	// Get Presentation clocks organised.
	CHECK_HR(MFCreatePresentationClock(&pClock), "Failed to create presentation clock.");
	CHECK_HR(MFCreateSystemTimeSource(&pTimeSource), "Failed to create system time source.");
	CHECK_HR(pClock->SetTimeSource(pTimeSource), "Failed to set time source.");
	CHECK_HR(pVideoSink->SetPresentationClock(pClock), "Failed to set presentation clock on video sink.");
	CHECK_HR(pClock->Start(0), "Error starting presentation clock.");
	

	//iterate through the frameBuffer and copy to the render buffer
	{
		size_t transfered = 0; //create data tracker
		std::unique_lock<std::mutex> lock(m); //lock access to the frame buffer
		intermediateByteBuffer.resize(frameQueue.begin()->currentLength); //resize the buffer

		//copy all data from the RTP receiver to the frame queue
		for (int i = 0; i < frameQueue.begin()->frameData.size(); i++)
		{
			memcpy(&intermediateByteBuffer[transfered], frameQueue.begin()->frameData[i].payload.data(), frameQueue.begin()->frameData[i].payloadLength);
			transfered += frameQueue.begin()->frameData[i].payloadLength;
		}
		
		//extract the 10 bit values into WORDs with padding //todo optimise the loops
		int x = 0;
		for (int i = 0; i < intermediateByteBuffer.size(); i += 5)
		{
			intermediateWORDBuffer.resize(intermediateWORDBuffer.size()+4);
			intermediateWORDBuffer[1 + x] = (((uint16_t)intermediateByteBuffer[0 + i] & 0b11111111u) << (0 + 8)) | (((uint16_t)intermediateByteBuffer[1 + i] & 0b11000000u) << 0);
			intermediateWORDBuffer[0 + x] = (((uint16_t)intermediateByteBuffer[1 + i] & 0b00111111u) << (2 + 8)) | (((uint16_t)intermediateByteBuffer[2 + i] & 0b11110000u) << 2);
			intermediateWORDBuffer[3 + x] = (((uint16_t)intermediateByteBuffer[2 + i] & 0b00001111u) << (4 + 8)) | (((uint16_t)intermediateByteBuffer[3 + i] & 0b11111100u) << 4);
			intermediateWORDBuffer[2 + x] = (((uint16_t)intermediateByteBuffer[3 + i] & 0b00000011u) << (6 + 8)) | (((uint16_t)intermediateByteBuffer[4 + i] & 0b11111111u) << 6);
			x += 4;
		}
		renderBuffer.resize(intermediateWORDBuffer.size()); //resize the render buffer
		//convert 10 bits (16 with padding) to 8 bits //TODO optimise the loops
		for (int i = 0; i < intermediateWORDBuffer.size(); i += 8)
		{
			renderBuffer[i + 0] = (intermediateWORDBuffer[i + 0] >> 8);
			renderBuffer[i + 1] = (intermediateWORDBuffer[i + 1] >> 8);
			renderBuffer[i + 2] = (intermediateWORDBuffer[i + 2] >> 8);
			renderBuffer[i + 3] = (intermediateWORDBuffer[i + 3] >> 8);
			renderBuffer[i + 4] = (intermediateWORDBuffer[i + 4] >> 8);
			renderBuffer[i + 5] = (intermediateWORDBuffer[i + 5] >> 8);
			renderBuffer[i + 6] = (intermediateWORDBuffer[i + 6] >> 8);
			renderBuffer[i + 7] = (intermediateWORDBuffer[i + 7] >> 8);
		}
	};
	//set the sample time and duration for each frame
	CHECK_HR(pD3DVideoSample->SetSampleTime(llTimeStamp), "Failed to set D3D video sample time.");
	CHECK_HR(pD3DVideoSample->SetSampleDuration(sampleDuration), "Failed to set D3D video sample duration.");
	//move the data to the renderer and process the sample
	CHECK_HR(p2DBuffer->ContiguousCopyFrom((BYTE*) renderBuffer.data(), renderBuffer.size()), "Failed to copy frame to D2D buffer.");
	CHECK_HR(pStreamSink->ProcessSample(pD3DVideoSample), "Stream sink process sample failed.");

done:

	//release all the resources
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

	std::cin.get();
	return;
}

//threading function for the RTP receiver
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
				ProcessRTPPacket(*pack);
				DeletePacket(pack); //delete the memory once the packet has been proccessed
			}
			
		} while (GotoNextSourceWithData());
	}

	EndDataAccess();
}

//function to process the received packet
void Renderer::ProcessRTPPacket(const jrtplib::RTPPacket& rtppack)
{
	if (firstMarker) //check to see if we have receieved the first packet marker
	{
		if (rtppack.HasMarker()) //if received the first marker
		{
			addFrame(0); //add a frame to the frame queue
			firstMarker = false;
			goto done;
		}
		else { goto done; }
	} else
	{
		if (lastPacketHadMarker) //if this is a new frame
		{
			addFrame(rtppack.GetTimestamp()); //add frame to the frame queue
			lastPacketHadMarker = false;
		}
	}
	//add the payload to the frame data queue
	addPayload(rtppack.GetPayloadData(), rtppack.GetPayloadLength(), rtppack.GetTimestamp(), rtppack.GetExtendedSequenceNumber());
	if (rtppack.HasMarker()) { lastPacketHadMarker = true; } //if packet has marker then track it
done: return;
}


  




