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
Renderer::Renderer(unsigned __int32 width, unsigned __int32 height, unsigned __int32 bitDepth, HWND hwnd) : RTPSession(), BufferQueues(230400)
{
    sourceFormat.width = width;
    sourceFormat.height = height;
    sourceFormat.bitDepth = bitDepth;
	
    m_hwnd = hwnd;
}

void Renderer::addPayload()
{
	
}

void Renderer::addNewFrame()
{
	
}

void Renderer::startRTPReceiver(uint16_t port)
{
	WSAStartup(MAKEWORD(2, 2), &dat);

	sessparams.SetOwnTimestampUnit(1.0 / 8000.);
	transparams.SetPortbase(portbase);
	transparams.SetRTPReceiveBuffer(174483046);
	status = Create(sessparams, &transparams);
	checkRTPSessError(status);
	jrtplib::RTPTime::Wait(jrtplib::RTPTime(100, 0));

}

void Renderer::stopRTPReceiver()
{
	BYEDestroy(jrtplib::RTPTime(10, 0), 0, 0);
	WSACleanup();
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
			jrtplib::RTPSourceData* srcdat;

			srcdat = GetCurrentSourceInfo();

			while ((pack = GetNextPacket()) != NULL)
			{
				if (firstMarker && !pack->HasMarker())
				{
					goto notUsefullPacket;
				} else
				{
					ProcessRTPPacket(*srcdat, *pack);
				}
				
				
			}
			notUsefullPacket: DeletePacket(pack);
			
		} while (GotoNextSourceWithData());
	}

	EndDataAccess();
}

void Renderer::ProcessRTPPacket(const jrtplib::RTPSourceData& srcdat, const jrtplib::RTPPacket& rtppack)
{
	
}

    
  




