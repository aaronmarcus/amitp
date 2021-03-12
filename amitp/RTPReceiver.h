#pragma once

#ifndef RECEIVER_H
#define RECEIVER_H

#include "jrtplib3/rtpsession.h"
#include "jrtplib3/rtppacket.h"
#include "jrtplib3/rtpudpv4transmitter.h"
#include "jrtplib3/rtpipv4address.h"
#include "jrtplib3/rtpsessionparams.h"
#include "jrtplib3/rtperrors.h"
#include "jrtplib3/rtpsourcedata.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>

#include "Renderer.h" //to get access to the methods to dump payloads

class RTPReceiver : public jrtplib::RTPSession
{
public:

protected:
	void OnPollThreadStep();
	void ProcessRTPPacket(const jrtplib::RTPSourceData& srcdat, const jrtplib::RTPPacket& rtppack);

	Renderer* m_pRenderer;
};

#endif

