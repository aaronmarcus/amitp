#pragma once

#ifndef RENDERER_H
#define RENDERER_H

#include "RTPReceiver.h"
#include "MediaEventHandler.h"
#include "BufferQueue.h"
#include "commonTools.h"

#include <stdio.h>
#include <tchar.h>
#include <iostream>

#include <windowsx.h>

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
#include <Windows.h>
#include <windowsx.h>

#include <iostream>

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
struct SourceFormatDescriptor
{
	unsigned __int32 width;
	unsigned __int32 height;
	unsigned __int32 bitDepth;
	int fps;
	int sampling;
};


class Renderer : protected jrtplib::RTPSession, public BufferQueues
{
public:
	Renderer(unsigned __int32 width, unsigned __int32 height, unsigned __int32 bitDepth, HWND hwnd);

	void startRTPReceiver(uint16_t port);
	void stopRTPReceiver();
	void startRenderer();
	
protected:      
	
	//jrtplib
	void OnPollThreadStep() override;
	void ProcessRTPPacket(const jrtplib::RTPPacket& rtppack);
	WSAData dat;
	uint16_t portbase = 20000;
	int status;

	bool firstMarker;
	bool lastPacketHadMarker;
	
	
	jrtplib::RTPUDPv4TransmissionParams transparams;
	jrtplib::RTPSessionParams sessparams;
	
private:
	SourceFormatDescriptor sourceFormat;

	////Media Foudation
	IMFMediaType* pVideoOutType;
	IMFMediaSink* pVideoSink;
	IMFStreamSink* pStreamSink;
	IMFMediaTypeHandler* pSinkMediaTypeHandler;
	IMFVideoRenderer* pVideoRenderer;
	IMFVideoDisplayControl* pVideoDisplayControl;
	IMFGetService* pService;
	IMFActivate* pActive;
	IMFPresentationClock* pClock;
	IMFPresentationTimeSource* pTimeSource;
	IDirect3DDeviceManager9* pD3DManager;
	IMFVideoSampleAllocator* pVideoSampleAllocator;
	IMFSample* pD3DVideoSample;
	IMFMediaBuffer* pDstBuffer;
	IMF2DBuffer* p2DBuffer;
	RECT rc;
	BOOL fSelected;
	std::vector<BYTE> renderBuffer;
	IMFMediaEventGenerator* pEventGenerator;
	IMFMediaEventGenerator* pstreamSinkEventGenerator;
	MediaEventHandler mediaEvtHandler;
	MediaEventHandler streamSinkMediaEvtHandler;

	//handle to the window
	HWND m_hwnd;
};

#endif

