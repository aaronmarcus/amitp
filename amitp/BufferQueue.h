#pragma once

#ifndef BUFFERQUEUE_H
#define BUFFERQUEUE_H

#include <vector>

struct packetPayload
{
	std::vector<unsigned char> payload; //payload container
	uint32_t timestamp;
	uint32_t extSeqNum;
	size_t payloadLength;
};

struct Frame
{
	std::vector<packetPayload> frameData; //contains the payloads to make the frame
	uint32_t timestamp;
	bool frameComplete = false;
	
};

class BufferQueues
{
public:
	BufferQueues(size_t bufferSize);
protected:

	//frames
	std::vector<Frame> frameQueue;

	size_t m_bufferSize;

};

#endif
