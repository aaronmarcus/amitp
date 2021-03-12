#include "Renderer.h"

void checkerror(int rtperr)
{
	if (rtperr < 0)
	{
		std::cout << "ERROR: " << jrtplib::RTPGetErrorString(rtperr) << std::endl;
		exit(-1);
	}
}

//Renderer Constructor
Renderer::Renderer(unsigned __int32 width, unsigned __int32 height, unsigned __int32 bitDepth, HWND hwnd) : bufferQueue(230400)
{
    sourceFormat.width = width;
    sourceFormat.height = height;
    sourceFormat.bitDepth = bitDepth;
	
    m_hwnd = hwnd;
}

void Renderer::startRTPReceiver(uint16_t port)
{
	portbase = port;
	
    WSAStartup(MAKEWORD(2, 2), &dat);

	sessparams.SetOwnTimestampUnit(1.0 / 8000.0);

	transparams.SetPortbase(portbase);
	transparams.SetRTPReceiveBuffer(172483046); //1.3Gb - equivalent to a full second of packets
	status = rtpSess.Create(sessparams, &transparams);
	checkerror(status);
}

void Renderer::stopRTPReceiver()
{
	rtpSess.BYEDestroy(jrtplib::RTPTime(10, 0), 0, 0);
	WSACleanup();

}

    
  




